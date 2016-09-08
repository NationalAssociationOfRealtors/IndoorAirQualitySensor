use <roundedcube.scad>
module top_part(rows, columns, cell_step, inner_walls, outer_walls, height, lid_clearance) {
    translate([-(cell_step + outer_walls), 0, height + 2 * outer_walls])
    rotate([0,180,0])
    difference() {
        translate([-((2 * outer_walls) + lid_clearance),
                   -((2 * outer_walls) + lid_clearance),
                   height + outer_walls - lid_depth])
            cube([(columns - 1) * cell_step + 4 * outer_walls + 2 * lid_clearance,
                  (rows - 1) * cell_step * sqrt(3) / 2 + 4 * outer_walls + 2 * lid_clearance,
                  outer_walls + lid_depth]);
        bottom_part(rows,columns, cell_step, inner_walls + lid_clearance, outer_walls + lid_clearance, height);
    }
}


module honeycomb_row(cells, cell_step, inner_walls, height) {
    translate([0, 0, -height/2])
    linear_extrude(height=height) {
        for (j = [0 : cells - 1]) {
            translate([j * cell_step, 0, 0]) rotate([0,0,30])
            circle(r=(cell_step - inner_walls)/sqrt(3),$fn=6);
        }
    }
}


module full_honeycomb(rows, columns, cell_step, inner_walls, height) {
    for (i = [0 : rows - 1]) {
        translate([(i % 2) * cell_step / 2, cell_step * i * sqrt(3) / 2, 0])
        honeycomb_row(columns, cell_step, inner_walls, height);
    }
}

module clipped_honeycomb(rows, columns, cell_step, inner_walls, height) {
    intersection() {
        cube([(columns - 1) * cell_step, (rows - 1) * cell_step * sqrt(3) / 2, height]);
        full_honeycomb(rows, columns, cell_step, inner_walls, 3 * height);
    }
}

module bottom_part(rows, columns, cell_step, inner_walls, outer_walls, height) {
    translate([outer_walls, outer_walls, 0])
    difference(){
    difference() {
        translate([-outer_walls, -outer_walls, -outer_walls])
        roundedcube([(columns - 1) * cell_step + 2 * outer_walls,
              (rows - 1) * cell_step * sqrt(3) / 2 + 2 * outer_walls,
              height + outer_walls]);
        clipped_honeycomb(rows, columns, cell_step, inner_walls, 2 * height);
    }
    translate([-outer_walls, -outer_walls, -outer_walls])
     cube([(columns - 1) * cell_step + 2 * outer_walls,
              (rows - 1) * cell_step * sqrt(3) / 2 + 2 * outer_walls,
              height]);
    }
}

