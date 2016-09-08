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
use <includes/triangles.scad>
top_cover_height = 12;
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
rows          = 8;
columns       = 6;

// cell step is hole size between opposite hexagon walls plus inner wall thickness
cell_step     = 8;

// inner depth of the hexagonal boxes
height        = 3;

// walls thickness
inner_walls   = 2;
outer_walls   = .95;

// this clearance should allow fitting of the lid over the bottom box
lid_clearance = 0.6;

// how far does the lid protrube inside the bottom box
lid_depth     = 5;

////////////////////////////////////////////////
// Modules 
/////////////////////////////////////////////////

module top_slides(){
  translate([-1,1,0])  
  cube([1 , base_width+2*case_shell-2,1]);  
   
  translate([base_length+2*case_shell,1,0])  
  cube([1 , base_width+2*case_shell-2,1]);  
   
   
}


module inner_channel(){
  translate([2,1,top_cover_height-3])  
  color("red") cube([1 , base_width+2*case_shell-2,1]);  
   
  translate([base_length+2*case_shell-3,1,top_cover_height-3])  
  color ("blue")cube([1 , base_width+2*case_shell-2,1]);  
   
   
}

module build_bottom(){
  color("pink")translate([-2.5,0,1.25])  
  cube([2,base_width+2*case_shell,1]);  
  
  translate([base_length+2*case_shell+.5,0,1.25])  
  color("blue") cube([2 , base_width+2*case_shell,1]); 
    
  translate([-2.5,0,-1.5])  
  color("gray")cube([1 , base_width+2*case_shell,3.25]);  
    
  translate([base_length+2*case_shell+1.5,0,-1.5])  
  color("orange")cube([1 , base_width+2*case_shell,3.25]);    
    
  translate([base_length+2*case_shell+.5,base_width+2*case_shell-1,-1.5])  
  color("red")cube([1 , 1,3.25]);       
    
    
  translate([-1.5,base_width+2*case_shell-1,-1.5])  
  color("blue")cube([1 , 1,3.25]);       
    
  translate([-2.5,0,-1.5])  
  cube([base_length+2*case_shell+4.3, base_width+2*case_shell,1.25]);  
     
   
    
     
    
}

module top_cover() {
  difference() {
    cube([base_length + 2 * case_shell, base_width + 2 * case_shell, top_cover_height + 2 * case_shell]);
      translate([case_shell, case_shell, 0])
        cube([base_length, base_width, top_cover_height + 2 *case_shell]);
   }
  
}

module pin_outs() {
  pin_outs_x = .7;
  pin_outs_y = 1; 
  translate([case_shell, case_shell, case_shell])
    translate([.7, 1, top_cover_height]) {
      color("green") cube([14, 6, case_shell], false);
  }
}

module pin_outs_border() {
  pin_outs_x = 0;
  pin_outs_y = 0; 
  translate([case_shell, case_shell, case_shell])
    translate([0, 0, top_cover_height]) {
      color("green") cube([16, 8, case_shell], false);
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
  translate ([base_length /2 , base_width/2 -14 + spacer, case_shell])

      rotate([0,0,90])write("CRT Labs", font = font , h = 6, bold=2);
   
  translate ([base_length *2 /3, base_length/2 -2 , top_cover_height-3.5])
    rotate([0,0,90])
      write("crtlabs.org", font = font , h = 5);

}


module side_vent(){
    translate([0,10,0])
        cube([2,2,4]);
}


module build_top(){  
  translate ([0,0,top_cover_height + 2 * case_shell])
    rotate([180,0,0]){
      difference(){
        union(){
               top_cover();
               translate([1,1, top_cover_height +1])
               bottom_part(rows, columns, cell_step, inner_walls, outer_walls, height);
               top_slides();
               inner_channel();
        }     
    }
  }
}

module main(){
  
    build_top();
    translate([1,spacer,0])
    build_bottom();
    //add_text();
}

main();