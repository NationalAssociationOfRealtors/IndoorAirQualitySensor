// IAQ Cover and Base v.0.01
// August 2016
// Initial Design to Protect Sensor Board IAQ Circuit Board Designed by Akram Ali
// CRT Labs / National Association of REALTORS
// https://crtlabs.org

////////////////////////////////////////////////
// Includes and Definitions
////////////////////////////////////////////////
use <includes/writetext.scad>
use <includes/honeycomb.scad>
use <includes/sensorlocations.scad>
use <includes/roundedcube.scad>
top_cover_height = 5;
base_width = 47.5;
base_length = 40.5;
case_shell = 2;
spacer = 10;
font = "orbitron.dxf";

////////////////////////////////////////////////
// Honeycomb Settings
////////////////////////////////////////////////
// number of rows and columns, beware that some hexagonal cells are clipped
// at rectangular box boundaries, so the total number of cells will be
// smaller than rows * columns
rows          = 12;
columns       = 9;

// cell step is hole size between opposite hexagon walls plus inner wall thickness
cell_step     = 5.07;

// inner depth of the hexagonal boxes
height        = 3;

// walls thickness
inner_walls   = .75;
outer_walls   = 1;

// this clearance should allow fitting of the lid over the bottom box
lid_clearance = 0.6;

// how far does the lid protrube inside the bottom box
lid_depth     = 5;

////////////////////////////////////////////////
// Modules 
/////////////////////////////////////////////////
module top_cover() {
  difference() {
    roundedcube([base_length + 2 * case_shell, base_width + 2 * case_shell, top_cover_height + 2 * case_shell],false,.7);
      translate([case_shell, case_shell, 0])
        cube([base_length, base_width, top_cover_height + 2 *case_shell]);
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

module pin_outs() {
  pin_outs_x = .7;
  pin_outs_y = 1.25; 
  translate([case_shell, case_shell, case_shell])
    translate([.7, 1.25, top_cover_height]) {
      color("green") cube([12, 5.6, case_shell], false);
  }
}

module pin_outs_border() {
  pin_outs_x = 0;
  pin_outs_y = 0; 
  translate([case_shell, case_shell, case_shell])
    translate([0, 0, top_cover_height]) {
      color("green") cube([15, 9, case_shell], false);
  }
}
module cutouts(){
        microphone(); 
        reset_button();
        pin_outs();
        co_no2();
        light_intensity();
        pressure();
        temp_humid();
}
module add_text(){
  translate ([base_length + case_shell + 2, base_length/2 - 8, top_cover_height])
    rotate([90,0,90])
      write("CRT Labs IAQ Monitor ", font = font , h = 2);


  translate ([base_length + case_shell + 2, base_length-25, top_cover_height-2.75])
   rotate([90,0,90])
     write("https://crtlabs.org", font = font, h = 1.5);
     
}

module side_vent(){
    translate([0,10,0])
        cube([2,2,4]);
}
module main(){
    
translate ([0,0,top_cover_height + 2 * case_shell])
   rotate([180,0,0]){
        difference(){
           union(){
               top_cover();
                translate([1,1, top_cover_height +1])
                bottom_part(rows, columns, cell_step, inner_walls, outer_walls, height);
                pin_outs_border();
           }       
        pin_outs();
         //side_vent();
        }
  
        add_text();
             
        }
    
}

main();