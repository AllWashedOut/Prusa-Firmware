#ifndef MESH_BED_CALIBRATION_H
#define MESH_BED_CALIBRATION_H

// Exact positions of the print head above the bed reference points, in the world coordinates.
// The world coordinates match the machine coordinates only in case, when the machine
// is built properly, the end stops are at the correct positions and the axes are perpendicular.
extern const float bed_ref_points[] PROGMEM;

// Is the world2machine correction activated?
enum World2MachineCorrectionMode
{
	WORLD2MACHINE_CORRECTION_NONE  = 0,
	WORLD2MACHINE_CORRECTION_SHIFT = 1,
	WORLD2MACHINE_CORRECTION_SKEW  = 2,
};
extern uint8_t world2machine_correction_mode;
// 2x2 transformation matrix from the world coordinates to the machine coordinates.
// Corrects for the rotation and skew of the machine axes.
// Used by the planner's plan_buffer_line() and plan_set_position().
extern float world2machine_rotation_and_skew[2][2];
extern float world2machine_rotation_and_skew_inv[2][2];
// Shift of the machine zero point, in the machine coordinates.
extern float world2machine_shift[2];

// Resets the transformation to identity.
extern void world2machine_reset();
// Loads the transformation from the EEPROM, if available.
extern void world2machine_initialize();

// When switching from absolute to corrected coordinates,
// this will apply an inverse world2machine transformation
// to current_position[x,y].
extern void world2machine_update_current();

inline void world2machine(const float &x, const float &y, float &out_x, float &out_y)
{
	if (world2machine_correction_mode == WORLD2MACHINE_CORRECTION_NONE) {
		// No correction.
		out_x = x;
		out_y = y;
	} else {
		if (world2machine_correction_mode & WORLD2MACHINE_CORRECTION_SKEW) {
			// Firs the skew & rotation correction.
			out_x = world2machine_rotation_and_skew[0][0] * x + world2machine_rotation_and_skew[0][1] * y;
			out_y = world2machine_rotation_and_skew[1][0] * x + world2machine_rotation_and_skew[1][1] * y;
		}
		if (world2machine_correction_mode & WORLD2MACHINE_CORRECTION_SHIFT) {
			// Then add the offset.
			out_x += world2machine_shift[0];
			out_y += world2machine_shift[1];
		}
	}
}

inline void world2machine(float &x, float &y)
{
	if (world2machine_correction_mode == WORLD2MACHINE_CORRECTION_NONE) {
		// No correction.
	} else {
		if (world2machine_correction_mode & WORLD2MACHINE_CORRECTION_SKEW) {
			// Firs the skew & rotation correction.
			float out_x = world2machine_rotation_and_skew[0][0] * x + world2machine_rotation_and_skew[0][1] * y;
			float out_y = world2machine_rotation_and_skew[1][0] * x + world2machine_rotation_and_skew[1][1] * y;
			x = out_x;
			y = out_y;
		}
		if (world2machine_correction_mode & WORLD2MACHINE_CORRECTION_SHIFT) {
			// Then add the offset.
			x += world2machine_shift[0];
			y += world2machine_shift[1];
		}
	}
}

inline void machine2world(float x, float y, float &out_x, float &out_y)
{
	if (world2machine_correction_mode == WORLD2MACHINE_CORRECTION_NONE) {
		// No correction.
		out_x = x;
		out_y = y;
	} else {
		if (world2machine_correction_mode & WORLD2MACHINE_CORRECTION_SHIFT) {
			// Then add the offset.
			x -= world2machine_shift[0];
			y -= world2machine_shift[1];
		}
		if (world2machine_correction_mode & WORLD2MACHINE_CORRECTION_SKEW) {
			// Firs the skew & rotation correction.
			out_x = world2machine_rotation_and_skew_inv[0][0] * x + world2machine_rotation_and_skew_inv[0][1] * y;
			out_y = world2machine_rotation_and_skew_inv[1][0] * x + world2machine_rotation_and_skew_inv[1][1] * y;
		}
	}
}

inline void machine2world(float &x, float &y)
{
	if (world2machine_correction_mode == WORLD2MACHINE_CORRECTION_NONE) {
		// No correction.
	} else {
		if (world2machine_correction_mode & WORLD2MACHINE_CORRECTION_SHIFT) {
			// Then add the offset.
			x -= world2machine_shift[0];
			y -= world2machine_shift[1];
		}
		if (world2machine_correction_mode & WORLD2MACHINE_CORRECTION_SKEW) {
			// Firs the skew & rotation correction.
			float out_x = world2machine_rotation_and_skew_inv[0][0] * x + world2machine_rotation_and_skew_inv[0][1] * y;
			float out_y = world2machine_rotation_and_skew_inv[1][0] * x + world2machine_rotation_and_skew_inv[1][1] * y;
			x = out_x;
			y = out_y;
		}
	}
}

extern bool find_bed_induction_sensor_point_z(float minimum_z = -10.f);
extern bool find_bed_induction_sensor_point_xy();

// Positive or zero: ok
// Negative: failed
enum BedSkewOffsetDetectionResultType {
	// Detection failed, some point was not found.
	BED_SKEW_OFFSET_DETECTION_FAILED = -1,

	// Detection finished with success.
	BED_SKEW_OFFSET_DETECTION_PERFECT = 0,
	BED_SKEW_OFFSET_DETECTION_SKEW_MILD,
	BED_SKEW_OFFSET_DETECTION_SKEW_EXTREME,
	// Detection finished with success, but it is recommended to fix the printer mechanically.
	BED_SKEW_OFFSET_DETECTION_FRONT_LEFT_FAR,
	BED_SKEW_OFFSET_DETECTION_FRONT_RIGHT_FAR
};

extern BedSkewOffsetDetectionResultType find_bed_offset_and_skew(int8_t verbosity_level);
extern BedSkewOffsetDetectionResultType improve_bed_offset_and_skew(int8_t method, int8_t verbosity_level);


extern void reset_bed_offset_and_skew();
extern bool is_bed_z_jitter_data_valid();

// Scan the mesh bed induction points one by one by a left-right zig-zag movement,
// write the trigger coordinates to the serial line.
// Useful for visualizing the behavior of the bed induction detector.
extern bool scan_bed_induction_points(int8_t verbosity_level);

#endif /* MESH_BED_CALIBRATION_H */