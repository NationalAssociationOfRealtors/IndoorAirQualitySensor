// IAQ Cover and Base v.0.01
// August 2016
// Initial Design to Protect Sensor Board IAQ Circuit Board Designed by Akram Ali
// CRT Labs / National Association of REALTORS
// https://crtlabs.org

////////////////////////////////////////////////
// Includes and Definitions
////////////////////////////////////////////////
use <Write.scad>
top_cover_height = 1;
base_width = 47.5;
base_length = 40.5;
case_shell = 2;
spacer = 10;

////////////////////////////////////////////////
// Modules 
////////////////////////////////////////////////
module top_cover() {

  difference() {

      cube([base_length + 2 * case_shell, base_width + 2 * case_shell, top_cover_height + 2 * case_shell]);
      translate([case_shell, case_shell, 0])
      cube([base_length, base_width, top_cover_height + case_shell]);

    }
}

module bottom_cover() {
  translate([0, base_width + spacer, 0]) {
    difference() {
      cube([base_length + 2 * case_shell, base_width + 2 * case_shell, top_cover_height + 2 * case_shell]);
      translate([case_shell, case_shell, case_shell])
      cube([base_length, base_width, top_cover_height + case_shell]);
    }
  }
}


module microphone() {
  microphone_x = 5.5;
  microphone_y = 40;
  translate([case_shell, case_shell, case_shell])
  translate([microphone_x, microphone_y, top_cover_height]) {
    color("red") cylinder(h = case_shell, d1 = 7, d2 = 7);
  }
}

module reset_button() {
  reset_button_x = 29.7;
  reset_button_y = 2.34;
  translate([case_shell, case_shell, case_shell])
  translate([reset_button_x, reset_button_y, 0]) {
    color("blue") cylinder(h = case_shell * 2, d1 = 4, d2 = 4, center = false);
  }
}

module pin_outs() {
  pin_outs_x = .7;
  pin_outs_y = 1.25; 
  translate([case_shell, case_shell, case_shell])
  translate([.7, 1.25, top_cover_height]) {
    color("green") cube([12, 5.6, case_shell], false);
  }
}

module co_no2() {    
  co_no2_x = 16.9;
  co_no2_y = 20.5;
  translate([case_shell, case_shell, case_shell])
  translate([co_no2_x,  co_no2_y, top_cover_height]) {
    color("orange") cube([8, 5.5, case_shell], false);
  }
}

module pressure() {
  pressure_x = .5;
  pressure_y = 31;
  translate([case_shell, case_shell, case_shell])
  translate([pressure_x, pressure_y, top_cover_height]) {
    color("green") cube([3, 5, case_shell], false);
  }
}

module temp_humid() {  
  temp_humid_x = 17;
  temp_humid_y = 38;
  translate([case_shell, case_shell, case_shell])
  translate([temp_humid_x, temp_humid_y, top_cover_height]) {
    color("green") cube([2, 2, case_shell], false);
  }
}

module light_intensity() {
    
  light_intensity_x = 17;
  light_intensity_y = 35;
  translate([case_shell, case_shell, case_shell])
  translate([light_intensity_x, light_intensity_y, top_cover_height]) {
    color("green") cube([2, 2, case_shell], false);
  }
}


////////////////////////////////////////////////
// Top Cover
////////////////////////////////////////////////


//uncomment to flip top cover for easier printing
//translate([0,0,top_cover_height + 2* case_shell]){
//rotate([180,0,0]){    

difference() {
  top_cover(); //start with the top case
  microphone(); //subtract the rest of the cutouts
  reset_button();
  pin_outs();
  co_no2();
  light_intensity();
  pressure();
  temp_humid();
}

////////////////////////////////////////////////
// Bottom Cover
////////////////////////////////////////////////
bottom_cover();

////////////////////////////////////////////////
//Add Text
////////////////////////////////////////////////

translate ([base_width +case_shell-7, case_shell, top_cover_height+2*case_shell])
   rotate([0,0,90])
        write("CRT Labs IAQ", font = "orbitron.dxf", h = 2);


translate ([base_width +case_shell-7, base_length-10, top_cover_height + 2*case_shell])
   rotate([0,0,90])
        write("https://crtlabs.org", font = "orbitron.dxf", h = 1.5);