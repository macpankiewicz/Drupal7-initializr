<?php

/**
 * A genericized version of lighten/darken so negative values can be used
 * @param SassColour $color - the color to adjust
 * @param SassNumber $amount  the value to adjust by
 */
function sassy_compass__adjust_lightness($color, $amount) {
	return sassy_compass__adjust_color_value($color, 'lightness', $amount);
}

/**
 * Scales a color's lightness by some percentage.
 * If the amount is negative, the color is scaled darker, if positive, it is scaled lighter.
 * This will never return a pure light or dark color unless the amount is 100%.
 */
function sassy_compass__scale_lightness($color, $amount) {
	return sassy_compass__scale_color_value($color, 'lightness', $amount);
}

/**
 * A genericized version of saturate/desaturate so negative values can be used
 * @param SassColour $color - the color to adjust
 * @param SassNumber $amount  the value to adjust by
 */
function sassy_compass__adjust_saturation($color, $amount) {
	return sassy_compass__adjust_color_value($color, 'saturation', $amount);
}

/**
 * Scales a color's lightness by some percentage.
 * If the amount is negative, the color is scaled darker, if positive, it is scaled lighter.
 * This will never return a pure light or dark color unless the amount is 100%.
 */
function sassy_compass__scale_saturation($color, $amount) {
	return sassy_compass__scale_color_value($color, 'saturation', $amount);
}

/**
 * returns an IE hex string for a color with an alpha channel
 * suitable for passing to IE filters.
 */
function sassy_compass__ie_hex_str($color) {
	SassLiteral::assertType($color,  'SassColour');
	$alpha = round($color->alpha * 255);
	$alpha_str = str_pad(dechex($alpha), 2, '0', STR_PAD_LEFT);
	$col = $color->asHex(FALSE);
	return new SassString(strtoupper("##{$alpha_str}#{$col}"));
}

function sassy_compass__adjust_color_value($color, $attribute, $amount) {
	if (!is_object($color)) {
		$color = new SassColour($color);
	}
	if (is_object($amount)) {
		$amount = $amount->value;
	}
	$amount = preg_replace('/[^0-9\.\-]/', '', $amount);

	// ensure we have all attributes;
	$color->getRgb();
	$color->getHsl();
	$value = $color->$attribute;

	$color->$attribute = $value + $amount;

	// ensure conversion took place...
	switch ($attribute) {
		case 'red':
		case 'green':
		case 'blue':
			$color->rgb2hsl();
		default:
			$color->hsl2rgb();
	}
	return $color;
}

function sassy_compass__scale_color_value($color, $attribute, $amount) {
	if (!is_object($color)) {
		$color = new SassColour($color);
	}
	if (is_object($amount)) {
		$amount = $amount->value;
	}
	$amount = preg_replace('/[^0-9\.\-]/', '', $amount);

	// ensure we have all attributes;
	$color->getRgb();
	$color->getHsl();
	$value = $color->$attribute;

	$color->$attribute = ($amount > 0) ? $value + (100 - $value) * ($amount / 100) : $value + ($value * $amount / 100);

	// ensure conversion took place...
	switch ($attribute) {
		case 'red':
		case 'green':
		case 'blue':
			$color->rgb2hsl();
		default:
			$color->hsl2rgb();
	}
	return $color;
}