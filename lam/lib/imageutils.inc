<?php

namespace LAM\ImageUtils;

use GdImage;
use Imagick;
use LAMException;

/*

This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
Copyright (C) 2018 - 2026  Roland Gruber

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
 * Image manipulation functions.
 *
 * @author Roland Gruber
 */

/**
 * Factory to create modificators for images.
 *
 * @author Roland Gruber
 */
class ImageManipulationFactory {

	/**
	 * Returns an image manipulator based on installed PHP modules.
	 *
	 * @param string $imageData binary string of image data
	 * @return ImageManipulator manipulator
	 */
	public static function getImageManipulator($imageData) {
		if (extension_loaded('imagick')) {
			return new ImageManipulatorImagick($imageData);
		}
		return new ImageManipulatorGd($imageData);
	}

	/**
	 * Returns an image manipulator based on installed PHP modules.
	 *
	 * @param string $path path to image file
	 * @return ImageManipulator manipulator
	 */
	public static function getImageManipulatorFromFile($path) {
		$handle = fopen($path, "r");
		$data = fread($handle, 100000000);
		fclose($handle);
		return ImageManipulationFactory::getImageManipulator($data);
	}

}

/**
 * Modifies images.
 *
 * @author Roland Gruber
 */
interface ImageManipulator {

	/**
	 * Returns the height of the image.
	 *
	 * @return int height
	 */
	public function getHeight(): int;

	/**
	 * Returns the width of the image.
	 *
	 * @return int width
	 */
	public function getWidth(): int;

	/**
	 * Resizes the image to the given maximum dimensions.
	 *
	 * @param int $width width
	 * @param int $height height
	 */
	public function thumbnail(int $width, int $height): void;

	/**
	 * Converts the image to JPEG format.
	 */
	public function convertToJpeg(): void;

	/**
	 * Crops the image.
	 *
	 * @param int $x starting point in original image
	 * @param int $y starting point in original image
	 * @param int $width width of target size
	 * @param int $height height of target size
	 */
	public function crop(int $x, int $y, int $width, int $height): void;

	/**
	 * Returns the image as binary string.
	 *
	 * @return string image data
	 */
	public function getImageData(): string;

}

/**
 * Manipulates images using Imagick library.
 *
 * @author Roland Gruber
 */
class ImageManipulatorImagick implements ImageManipulator {

	/**
	 * Image
	 *
	 * @var Imagick image
	 */
	private $image;

	/**
	 * Constructor.
	 *
	 * @param string $imageData original image as binary string
	 */
	public function __construct($imageData) {
		$this->image = new Imagick();
		$this->image->readimageblob($imageData);
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::getHeight()
	 */
	public function getHeight(): int {
		return $this->image->getimageheight();
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::getWidth()
	 */
	public function getWidth(): int {
		return $this->image->getimagewidth();
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::getAsJpeg()
	 */
	public function convertToJpeg(): void {
		$this->image->setImageCompression(Imagick::COMPRESSION_JPEG);
		$this->image->setImageFormat('jpeg');
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::crop()
	 */
	public function crop(int $x, int $y, int $width, int $height): void {
		$this->image->cropimage((int) round($width), (int) round($height), (int) round($x), (int) round($y));
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::resize()
	 */
	public function thumbnail(int $width, int $height): void {
		if (($this->getWidth() <= $width) && ($this->getHeight() <= $height)) {
			// skip if smaller than target size
			return;
		}
		$this->image->thumbnailimage($width, $height, true);
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::getImageData()
	 */
	public function getImageData(): string {
		return $this->image->getimageblob();
	}

}

/**
 * Manipulates images using gd library.
 *
 * @author Roland Gruber
 */
class ImageManipulatorGd implements ImageManipulator {

	/**
	 * Image
	 *
	 * @var GdImage|false image
	 */
	private $image;

	/**
	 * GD image type
	 *
	 * @var int image type
	 */
	private $type;

	/**
	 * Constructor.
	 *
	 * @param string $imageData original image as binary string
	 * @throws LAMException error reading image
	 */
	public function __construct($imageData) {
		$this->image = @imagecreatefromstring($imageData);
		if ($this->image === false) {
			throw new LAMException(_('Photo'), _('Unable to read image'));
		}
		$info = getimagesizefromstring($imageData);
		$this->type = $info[2];
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::getHeight()
	 */
	public function getHeight(): int {
		return imagesy($this->image);
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::getWidth()
	 */
	public function getWidth(): int {
		return imagesx($this->image);
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::getAsJpeg()
	 */
	public function convertToJpeg(): void {
		$this->type = IMAGETYPE_JPEG;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::crop()
	 */
	public function crop(int $x, int $y, int $width, int $height): void {
		$this->image = imagecrop($this->image, [
			'x' => $x,
			'y' => $y,
			'width' => $width,
			'height' => $height
		]);
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::resize()
	 */
	public function thumbnail(int $width, int $height): void {
		if (($this->getWidth() <= $width) && ($this->getHeight() <= $height)) {
			// skip if smaller than target size
			return;
		}
		$thumbWidth = $this->getWidth();
		$thumbHeight = $this->getHeight();
		if ($thumbWidth > $width) {
			$factor = $width / $thumbWidth;
			$thumbWidth *= $factor;
			$thumbHeight *= $factor;
		}
		if ($thumbHeight > $height) {
			$factor = $height / $thumbHeight;
			$thumbWidth *= $factor;
			$thumbHeight *= $factor;
		}
		$thumbnail = imagecreatetruecolor($thumbWidth, $thumbHeight);
		imagecopyresampled(
			$thumbnail,
			$this->image,
			0, 0, 0, 0,
			$thumbWidth,
			$thumbHeight,
			$this->getWidth(),
			$this->getHeight());
		$this->image = $thumbnail;
	}


	/**
	 * {@inheritDoc}
	 * @see \LAM\ImageUtils\ImageManipulator::getImageData()
	 */
	public function getImageData(): string {
		ob_start();
		if ($this->type == IMAGETYPE_JPEG) {
			imagejpeg($this->image);
		}
		elseif ($this->type == IMAGETYPE_PNG) {
			imagepng($this->image);
		}
		$output = ob_get_contents();
		ob_clean();
		return $output;
	}

}
