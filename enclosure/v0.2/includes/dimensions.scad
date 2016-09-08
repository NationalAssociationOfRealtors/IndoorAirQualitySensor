

////////////////////////////////////////////////
// Box Dimensions Settings
////////////////////////////////////////////////
top_cover_height = 2;
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