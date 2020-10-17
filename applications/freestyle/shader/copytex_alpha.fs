#version 330
in vec2 tc;
uniform sampler2D tex;
uniform sampler2D rendered_diff;
uniform vec2 screenres = vec2(1, 1);
out vec4 out_col;

float map_val(float in_val, float input_start, float input_end, float output_start, float output_end) {
	return output_start + ((output_end - output_start) / (input_end - input_start)) * (in_val - input_start);
}

void main() {
	float outer_1 = screenres.x - screenres.y;
	float outer_2 = screenres.y - screenres.x;
	out_col = vec4(0, 0, 0, 0);
	if (outer_1 > 0.f) {
		vec2 texture_coord = tc;
		float aspect = screenres.x / screenres.y;
		float x_offset = 0.5f * (aspect - 1);
		texture_coord.x = map_val(tc.x, 0, 1, -x_offset, aspect - x_offset);
		if (texture_coord.x > 0 && texture_coord.x < 1) {
			out_col = texture(tex, texture_coord);
			out_col.rgb = vec3(1) - texture(rendered_diff, tc).rgb;
		} else { discard; }
		if (out_col.a < 0.8) {
			discard;
		}
		return;
	} else {
		vec2 texture_coord = tc;
		float aspect = screenres.y / screenres.x;
		float y_offset = 0.5f * (aspect - 1);
		texture_coord.y = map_val(tc.y, 0, 1, -y_offset, aspect - y_offset);
		if (texture_coord.y > 0 && texture_coord.y < 1) {
			out_col = texture(tex, texture_coord);
			out_col.rgb = vec3(1) - texture(rendered_diff, tc).rgb;
		} else { discard; }
		if (out_col.a < 0.8) {
			discard;
		}
		return;
	}
}
