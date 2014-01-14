<?php
interface IUserAgentDetector {
/**
 * @return bool
 */	
	public function IsMobile();
/**
 * @return bool
 */	
	public function IsTablet();
	
/**
 * @return bool
 */	
	public function Is($type);
}