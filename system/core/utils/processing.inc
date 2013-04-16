<?php
interface IDataFilter {
	/**
	 * @return string
	 */
	public function CallName();
	/**
	 * @param string $content
	 * @return string
	*/
	public function Filter($content);
}