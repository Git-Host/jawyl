<?php

class TLink {
/**
 * @var string
 */	
	public $Text;
/**
 * @var string
 */	
	public $Link;
/**
 * @param string $text
 * @param string $link
 */	
	public function __construct($text,$link){
		$this->Text = $text;
		$this->Link = $link;
	}
}

class TMenuLink extends TLink {
/**
 * @var TMenuLink[]
 */	
	public $Children = array();
	public function AddChild(TMenuLink $link){
		$this->Children[] = $link;
	}		
}

interface ITemplatePage {
/**
 * @return string
 */	
	public function Title();
/**
 * @return string
 */	
	public function Description();
/**
 * @return string
 */	
	public function KeyWords();
/**
 * @return string
 */	
	public function ContentsPage();
/**
 * @return integer
 */	
	public function ContentsPagesCount();
/**
 * @return string[]
 */	
	public function ContentsPages();
/**
 * @param integer $index
 * @return string
 */	
	public function ContentsPageLink($index);
/**
 * @return string
 */	
	public function NextContentsPageLink();
/**
 * @return string
 */	
	public function PreviosContentsPageLink();
/**
 * @return TLink[]
 */	
	public function NextPageLink();
/**
 * @return TLink[]
 */	
	public function PreviousPageLink();
/**
 * @return TLink[]
 */	
	public function Breadcrumbs();
/**
 * @return TLink[]
 */	
	public function Children();
/**
 * @return TMenuLink[]
 */	
	public function Menu();
/**
 * @return string
 */	
	public function Contents();
/**
 * @return TWidget[]
 */	
	public function Widgets();
/**
 * @param string $name
 * @return TWidget
 */	
	public function Widget($name);
/**
 * @return string
 */	
	public function Meta();
}