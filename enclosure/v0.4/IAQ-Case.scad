// IAQ Cover and Base v0.3
// Sept 2016
// Initial Design to Protect Sensor Board IAQ Circuit Board Designed by Akram Ali
// CRT Labs / National Association of REALTORS
// https://crtlabs.org
////////////////////////////////////////////////
// Includes and Definitions
////////////////////////////////////////////////

top_cover_height = 7;
base_width = 50;
base_length = 40.5;
case_shell = 1.2;
case_shell_buffer = 2 * case_shell;
spacer = 10;

////////////////////////////////////////////////
// Modules 
/////////////////////////////////////////////////

module top_slides() {
    translate([-1, 1, 0])
    cube([1, base_width + 2 * case_shell - 2, 1]);

    translate([base_length + 2 * case_shell, 1, 0])
    cube([1, base_width + 2 * case_shell - 2, 1]);


}


module inner_channel() {
    translate([case_shell, 1, top_cover_height - 3])
    color("red") cube([1, base_width + 2 * case_shell - 20, 1]);

    translate([base_length + (2 * case_shell - 2), 1, top_cover_height - 3])
    color("blue") cube([1, base_width + 2 * case_shell - 2, 1]);


}
module outer_channel_cutout() {
    translate([0, 0, 3])
    cube([.6, base_width+2 * case_shell, 1]);

    //translate([base_length + (2 * case_shell), 0, 3])
    //cube([.6, base_width+2 * case_shell, 1]);


}
module build_bottom() {

    difference() {
        build_bottom_parts();
        bottom_cutouts();
        translate([-3, 0, -2])
        usb_cutout();

    }
}

module build_bottom_parts() {
    color("pink") translate([-2.5, 0, 1.4])
    cube([2, base_width + 2 * case_shell, 1]);

    translate([base_length + 2 * case_shell + .75, 0, 1.4])
    color("blue") cube([2, base_width + 2 * case_shell, 1]);

    translate([-2.5, 0, -1.5])
    color("gray") cube([1, base_width + 2 * case_shell, 3.25]);

    translate([base_length + 2 * case_shell + 1.75, 0, -1.5])
    color("orange") cube([1, base_width + 2 * case_shell, 3.25]);

    translate([-2.5, 0, -1.5])
    cube([base_length + 2 * case_shell + 4.3, base_width + 2 * case_shell, 1.25]);

}

module microphone() {
    microphone_x = 5;
    microphone_y = 37;
    translate([case_shell, case_shell, -2])
    translate([microphone_x, microphone_y, 0]) {
        color("red") cylinder(h = case_shell+2, d1 = 4, d2 = 4);
    }
}

module walls() {
    difference() {
        cube([base_length + 2 * case_shell, base_width + 2 * case_shell, top_cover_height + 2 * case_shell]);
        translate([case_shell, case_shell, 0])
        cube([base_length, base_width, top_cover_height + 2 * case_shell]);
    }

}

module top_solid_cover() {
    translate([0, 0, top_cover_height + case_shell])
    cube([base_length + 2 * case_shell, base_width / 2, case_shell]);

}

module top_traces() {
    ////translate([0, 0, top_cover_height +  case_shell+1])
    cube([base_length + 2 * case_shell, base_width / 2, case_shell]);

}


module top_cutouts() {
    usb_cutout();
    antenna();
    //outer_channel_cutout(); 

}

module cover_art() {
    
   //linear_extrude(height = 1, center = true, convexity = 10)
   //import (file = "includes/case.dxf");
    
    trace_thickness=3;
    via_diameter=5;
    via_cutout=1.5;
    ceiling = top_cover_height + case_shell;
    
    translate([0, (base_width + case_shell) *55/100, top_cover_height + case_shell])
      color("red")  cube([base_length*4/5, trace_thickness, case_shell]);
    
      
    translate([base_length*4/5, (base_width + case_shell) *55/100, top_cover_height + case_shell])
      color("red")
        rotate(45,0,0)
            cube([12.5, trace_thickness, case_shell]);
    
    
    translate([base_length-via_diameter/2+case_shell_buffer, (base_width + case_shell) *75/100, ceiling])
     difference(){
        color("red") cylinder(h = case_shell, d1 = via_diameter, d2 = via_diameter);
           cylinder(h = case_shell, d1 = via_cutout, d2 = via_cutout);
     }
    
    translate([base_length/5+ case_shell_buffer, (base_width + case_shell) *9/10, ceiling])
    color("blue")cube([base_length*4/5,trace_thickness, case_shell]);

    translate([via_diameter/2, (base_width + case_shell) *75/100, top_cover_height + case_shell])
     difference(){
        color("blue") cylinder(h = case_shell, d1 = via_diameter, d2 = via_diameter);
           cylinder(h = case_shell, d1 = via_cutout, d2 = via_cutout);
     }

  translate([via_diameter-case_shell, (base_width + case_shell) *75/100 -.4, ceiling])
      color("blue")
        rotate(45,0,0)
            cube([12.5, trace_thickness, case_shell]);

translate([4,(base_width + case_shell) *55/100, ceiling])
      color("green")
        rotate(45,0,0)
            cube([25, trace_thickness, case_shell]);
     
     translate([14,(base_width + case_shell) *55/100, ceiling])
      color("orange")
        rotate(45,0,0)
            cube([25, trace_thickness, case_shell]);
     
        translate([24,(base_width + case_shell) *55/100, ceiling])
      color("orange")
        rotate(45,0,0)
            cube([25, trace_thickness, case_shell]);
     

}

module antenna() {
  antenna_x = 3.85;
  antenna_y = base_width;
  translate([case_shell, case_shell*2, 0])
    translate([antenna_x, antenna_y, 4.5]) {
      color("red") 
        rotate([90,0,0])
            cylinder(h = case_shell, d1 = 7.1, d2 = 7.1);
  }
}

module bottom_cutouts() {

    microphone();
}

module usb_cutout() {
    translate([0, 10, 3])
    cube([5, 13, 4]);
}


module build_top() {
    difference() {
        build_top_parts();
        top_cutouts();
    }

}

module build_top_parts() {


    walls();
    //  translate([1,1, top_cover_height +1])
    // bottom_part(rows, columns, cell_step, inner_walls, outer_walls, height);
    top_slides();
    inner_channel();
    top_solid_cover();
    cover_art();


}

module main() {
    //translate ([0,0,top_cover_height + 2 * case_shell])
    //rotate([180,0,0]){
    //difference(){
    build_top();
    
    //  top_cutouts();
    //}
    // translate([1,spacer,0])

    build_bottom();

}


//add_text();
//}


main();