use <../IAQ-Case.scad>



top_cover_height = 2;
base_width = 47.5;
base_length = 40.5;
case_shell = 2;
spacer = 10;
font = "orbitron.dxf";

module microphone() {
  microphone_x = 5.5;
  microphone_y = 41;
  translate([case_shell, case_shell, case_shell])
    translate([microphone_x, microphone_y, top_cover_height]) {
      color("red") cylinder(h = case_shell, d1 = 8, d2 = 8);
  }
}

module reset_button() {
  reset_button_x = 30.55;
  reset_button_y = 2.6;
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
  pressure_x = 1.5;
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
      color("green") cube([3, 3, case_shell], false);
  }
}

module light_intensity() {
    
  light_intensity_x = 17;
  light_intensity_y = 35;
  translate([case_shell, case_shell, case_shell])
    translate([light_intensity_x, light_intensity_y, top_cover_height]) {
      color("green") cube([3, 3, case_shell], false);
  }
}
