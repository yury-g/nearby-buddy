#pragma once
#include <stdint.h>
#include <math.h>

enum class Mood : uint8_t { Sleep, Sad, Smile, Happy, Elated, Dance };

inline const char *moodName(Mood mood) {
  constexpr const char *names[] = {"sleep", "sad", "smile", "happy", "elated", "dance"};
  return names[static_cast<unsigned>(mood)];
}

// Indoor starting values. Use a known-distance sweep to calibrate each receiver.
inline float estimatedFeet(float rssi) {
  constexpr float referenceAtOneMeter = -64.0f;
  constexpr float pathLossExponent = 2.2f;
  return 3.28084f * powf(10.0f, (referenceAtOneMeter - rssi) / (10.0f * pathLossExponent));
}

class Proximity {
 public:
  static constexpr uint32_t staleMs = 1100;
  static constexpr uint32_t sleepAfterMs = 1800;

  void observe(int8_t rssi, uint32_t now) {
    if (!seen_ || now - heardAt_ >= staleMs) {
      filtered_ = rssi;
      candidate_ = mood_;
      proposed_ = 0;
    } else {
      // Weight by elapsed time so duplicate reports do not accelerate the filter.
      const float elapsed = now - heardAt_;
      filtered_ += elapsed / (220.0f + elapsed) * (rssi - filtered_);
    }
    heardAt_ = now;
    seen_ = true;
    ++samples_;
  }

  Mood update(uint32_t now) {
    if (!seen_ || age(now) >= staleMs) {
      if (mood_ > Mood::Sad) commit(Mood::Sad, now);
      candidate_ = mood_;
      proposed_ = 0;
    } else if (samples_ != evaluated_) {
      evaluated_ = samples_;
      const Mood next = classify();
      if (next != candidate_) {
        candidate_ = next;
        candidateAt_ = now;
        proposed_ = 0;
      }
      if (next != mood_) {
        ++proposed_;
        const uint32_t dwell = next == Mood::Dance ? 260 : next > mood_ ? 130 : 340;
        if (proposed_ >= 2 && now - candidateAt_ >= dwell) commit(next, now);
      }
    }
    if (mood_ == Mood::Sad && candidate_ <= Mood::Sad && now - changedAt_ >= sleepAfterMs) {
      commit(Mood::Sleep, now);
    }
    return mood_;
  }

  Mood mood() const { return mood_; }
  float filtered() const { return filtered_; }
  uint32_t age(uint32_t now) const { return seen_ ? now - heardAt_ : UINT32_MAX; }

 private:
  Mood classify() const {
    // First-trial values in dBm. Replace using both receivers' physical trials.
    constexpr int enter[] = {-72, -63, -54, -41};
    constexpr int hysteresis[] = {4, 4, 4, 5};
    for (int i = 3; i >= 0; --i) {
      const auto band = static_cast<Mood>(i + static_cast<int>(Mood::Smile));
      const int limit = enter[i] - (mood_ >= band ? hysteresis[i] : 0);
      if (filtered_ >= limit) return band;
    }
    return mood_ == Mood::Sleep ? Mood::Sleep : Mood::Sad;
  }

  void commit(Mood next, uint32_t now) {
    mood_ = candidate_ = next;
    changedAt_ = now;
    proposed_ = 0;
  }

  Mood mood_ = Mood::Sad;
  Mood candidate_ = Mood::Sad;
  uint32_t heardAt_ = 0;
  uint32_t changedAt_ = 0;
  uint32_t candidateAt_ = 0;
  uint32_t samples_ = 0;
  uint32_t evaluated_ = 0;
  unsigned proposed_ = 0;
  float filtered_ = 0;
  bool seen_ = false;
};
