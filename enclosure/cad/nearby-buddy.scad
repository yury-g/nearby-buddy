// Nearby Buddy enclosure v0.1 — millimeters. MIT, Yury Gitman 2026.
// See ../DIMENSIONS.md for vendor dimensions and design allowances.
// Export parts in their supplied print orientations. No generated supports.
part = "assembly"; // shell, carrier, lid, key, assembly, exploded, cutaway, envelope
show_board = true;
show_battery = true;
larger_battery = false;
$fn = 48;

// Larger cells are limited by physical size, connector and electrical ratings.
battery_target = [50,25,10];
battery_max_3898 = [37,17.5,8.2];
bay = [52,27,12];
bay_origin = [14,-2,4.5];
pcb_front = 27;
pcb_thickness = 1.57;
deck_z = 17;
deck_t = 1.5;
rail_clearance = 0.25;
key_clearance = 0.25;
body_right = bay_origin[0]+bay[0]+6; // 72
key_x = body_right-2;
rail_y = [-9,28];
mounts = [[2.54,2.54],[2.54,20.32],[48.26,1.905],[48.26,21.0185]];
// Right mounts above are Eagle y values reflected around 22.86.
reset_xy = [10.795,22.86-5.334];

module rounded_box(x,y,z,w,d,h,r=2) {
    translate([x,y,z]) linear_extrude(h)
        offset(r=r) offset(delta=-r) square([w,d]);
}
// X-extruded trapezoid. Each shoulder is 45 degrees, printable both ways.
module rail(x0,len,y,c=0) {
    translate([x0,y,0]) rotate([0,90,0]) linear_extrude(len)
        polygon([[-1.55+c,-1-c],[-3.6-c,-3-c],[-3.6-c,3+c],[-1.55+c,1+c]]);
}
module key_hole() {
    translate([key_x,40,6]) rotate([90,0,0]) linear_extrude(60)
        rotate(45) square([2.6+key_clearance*2,2.6+key_clearance*2],center=true);
}
module shell() {
    difference() {
        union() {
            difference() {
                rounded_box(-5,-14,0,body_right+5,47,29,4);
                // Rear cavity leaves a 2 mm front bezel.
                rounded_box(-3,-8,-1,body_right+1,35,28,2);
                // A single wide face aperture exposes LCD, D0/D1/D2 and RESET.
                translate([4.7,1.1,26.9]) cube([42.5,20.7,4]);
            }
            // Integral strap eyelet, printed on the same face-down bed plane.
            difference() {
                rounded_box(body_right-2,4,24,11,12,5,3);
                translate([body_right+4.5,10,23]) cylinder(h=7,d=4);
            }
            // Carrier front stops. Ramp grows <=45 degrees in face-down print.
            for(x=[-2,53]) for(side=[0,1])
                translate([x,side ? 27.5 : -8.5,18.5]) scale([1,side ? -1 : 1,1])
                    rotate([0,90,0]) linear_extrude(3)
                        polygon([[0,0],[-(side ? 3 : 5),0],[0,side ? 3 : 5]]);
        }
        // Left USB opening includes connector body and plug insertion space.
        translate([-6,0,0]) rotate([0,90,0]) linear_extrude(10)
            polygon([[-26.6,5.2],[-26.6,17.7],[-21.1,17.7],[-14.85,11.45],[-21.1,5.2]]);
        // 45-degree captive slide rails with 0.25 mm clearance per side.
        for(y=rail_y) rail(-3,body_right+4,y,rail_clearance);
        translate([-3,-10.3,-1]) cube([body_right+7,39.6,2.85]);
        // Rear loading mouth for the lid and its lock bar.
        translate([body_right-5.3,-8.1,-1]) cube([8,35.2,12]);
        key_hole();
    }
}
module carrier() {
    difference() {
        union() {
            rounded_box(-2.6,-7.6,deck_z,body_right+0.2,34.2,deck_t,1.5);
            // Edge fence locates the PCB with 0.3 mm lateral clearance.
            difference() {
                rounded_box(-1.5,-1.5,deck_z,53.8,25.86,pcb_front-deck_z-0.1,4.04);
                rounded_box(-0.3,-0.3,deck_z-1,51.4,23.46,12,2.84);
                translate([-2,5.1,20.9]) cube([10,12.7,7]);
            }
            // Four solid stand-offs touch only the PCB mounting annuli.
            for(p=mounts) translate([p[0],p[1],deck_z])
                cylinder(h=pcb_front-pcb_thickness-deck_z-0.15,r=1.6);
        }
        // Wire / JST service bay. No battery wire crosses a closing seam.
        translate([4,-8,deck_z-1]) cube([13,15,12]);
        // Second (component-side) RESET remains reachable with a blunt tool.
        translate([reset_xy[0],reset_xy[1],deck_z-1]) cylinder(h=4,d=5);
        // Solid pads support mounting annuli; no screws or press-fit pins.
    }
}
module lid() {
    difference() {
        union() {
            translate([-2.8,-10.1,0]) cube([body_right+3.9,39.2,1.6]);
            for(y=rail_y) rail(-2.7,body_right+3.7,y);
            // Tall end bar stays outside both PCB and battery envelopes.
            translate([body_right-5,-7.7,1.4]) cube([6,34.4,8.6]);
            // Supports under the carrier lie outside the battery bay.
            for(x=[0,body_right-5]) for(y=[-5,23.5])
                translate([x,y,1.5]) cube([2.2,1.5,deck_z-1.65]);
        }
        key_hole();
        translate([reset_xy[0],reset_xy[1],-1]) cylinder(h=4,d=5);
    }
}
// Lock pin prints vertically on its broad head (a brim is useful).
// Head has a cord hole: a short retaining loop prevents accidental loss.
module key_print() {
    difference() {
        union() {
            rounded_box(-4.5,-4.5,0,9,9,2,2);
            translate([0,0,2]) cylinder(h=49,d=2.6*sqrt(2),$fn=4);
        }
        translate([2.9,2.9,-1]) cylinder(h=4,d=2);
    }
}
module key_assembled() {
    translate([key_x,-16,6]) rotate([-90,0,0]) key_print();
}
module board() {
    // Proper rigid rotation: component side down, display side up.
    translate([0,22.86,pcb_front]) rotate([180,0,0])
        import("../reference/board.stl",convexity=12);
}
module battery(large=false) {
    b=large ? battery_target : battery_max_3898;
    translate([large ? 15 : 14.5,large ? -1 : 0,16.5-b[2]]) cube(b);
}
module wire_route() {
    pts=[[14.5,8.75,16],[10.8,5,15.5],[10.8,-6.8,20],[10.8,-6.8,22]];
    for(i=[0:len(pts)-2]) hull() for(p=[pts[i],pts[i+1]]) translate(p) sphere(d=1.6,$fn=16);
}
module assembly(explode=0,cut=false) {
    color("#4953a6",cut ? 0.22 : 1) translate([0,0,explode*1.8]) difference() {
        shell();
        if(cut) translate([-8,-12,-1]) cube([body_right+20,20,32]);
    }
    color("#a7bdca") translate([0,0,0]) difference() {
        carrier();
        if(cut) translate([-8,-12,16.9]) cube([body_right+20,20,2]);
    }
    if(!cut) color("#2b3359") translate([0,0,-explode*2]) lid();
    if(!cut) color("#ffbd59") translate([0,-explode,0]) key_assembled();
    if(show_board) color("#6c497f") translate([0,0,explode*2.4]) board();
    if(show_battery) color("#d4d9dc") translate([0,0,-explode*0.8]) battery(larger_battery);
    if(show_battery && !explode) color("#e54553") wire_route();
}
module coupon() {
    translate([-20,-5,6]) rotate([180,0,0]) intersection() {
        shell(); translate([20,-14,0]) cube([12,9,6]);
    }
    translate([-20,25,0]) intersection() {
        lid(); translate([20,-12.5,0]) cube([12,7.5,4]);
    }
    difference() {
        translate([20,0,0]) cube([10,12,6]);
        translate([25-key_x,0,-3]) key_hole();
    }
}
if(part=="joint-coupon") coupon();
else if(part=="shell") translate([0,0,29]) rotate([180,0,0]) shell();
else if(part=="carrier") translate([0,0,-deck_z]) carrier();
else if(part=="lid") lid();
else if(part=="key") key_print();
else if(part=="shell_assembly") shell();
else if(part=="carrier_assembly") carrier();
else if(part=="lid_assembly") lid();
else if(part=="key_assembly") key_assembled();
else if(part=="board") board();
else if(part=="battery") battery(larger_battery);
else if(part=="wires") wire_route();
else if(part=="plug") translate([6.3,-6.8,20.8]) cube([9,7,6]);
else if(part=="bay") translate(bay_origin) cube(bay);
else if(part=="exploded") assembly(20);
else if(part=="cutaway") assembly(0,true);
else if(part=="envelope") {assembly(0,true); color("orange",0.3) translate(bay_origin) cube(bay);}
else assembly();
