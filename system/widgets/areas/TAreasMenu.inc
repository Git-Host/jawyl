<?php
/**
 * @package View
 * @subpackage Widgets
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
require_once 'common/presenter/basic.inc';
/**
 * @ignore
 */
require_once 'common/widget/lists.inc';
/**
 * @ignore
 */
require_once 'widgets/areas/TAreasWidget.inc';

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * menu widget
 * @property array $Items
 * @property TMenuItem $Current
 * @property IItemNavigation $ItemNavigation 
 */
class TAreasMenu extends TAreasWidget implements IMenu {
	private $_items_;
	private $_current_;
	
	private static $_menu_script_loaded_ = false;

/**
 * @var IItemNavigation
 */	
	protected $_ioc_item_navigator_;
	
	protected $_bool_popup_by_click_ = false;
	
	public function __get($nm){
		switch ($nm){
			case "Items":return $this->Items();break;
			case "Current":return $this->CurrentItem();break;
		}			
		return parent::__get($nm);
	}
	
	protected function checkSource($src){
		return ($src instanceof IMenuStructure) && parent::checkSource($src);
	}
	
/**
 * gets currently selected menu item
 * @return TMenuItem
 */	
	public function CurrentItem(){
		$this->Items();
		return $this->_current_;
	}
/**
 * gets menu top level items
 * @return array 
 */	
	public function Items(){
		if (is_null($this->_items_)){
			$this->_items_ = array();
			$sources = $this->Sources;
			foreach ($sources as $name=>$source){
				$temp = $source->Roots();
				foreach ($temp as $item){
					$l = $this->Wrap($item);
					if ($l instanceof INavigable)
						if ($l->Link() == $this->Application()->Uri())
							$this->_current_ = $l;
					$this->_items_[] = $l;
				}
			}
		} 
		return $this->_items_;
	}
	
	protected function onReloadEvent(TDataReloadEvent $event){
		$this->refreshArea($this->GetArea($this,"Container"));
	}
		
	public function SelectionLink(TMenuItem $item){
		if ($item instanceof TActionMenuItem)
			return $item->Action()->CallLink();
		return null; 
	}

	public function Wrap($base){
		if ($base instanceof ILink)
			return new TLinkMenuItem($base,$this, $this->Application());
		if ($base instanceof IAction)
			return new TActionMenuItem($base,$this);
		if ($base instanceof IItem){
			if ($this->ItemNavigator instanceof IItemNavigation)
				return new TNavigationMenuItem($base,$this,$this->ItemNavigator);
		}
		return new TMenuItem($base,$this);		
	}

/**
 * @see TWidget::DefaultOutput()
 */	
	protected function defaultOutput(IThemeEngine $engine){
		?>
		<div id="<?php echo $this->Name(); ?>" class="areas-menu">
		<?php
		$this->RenderArea($this->GetArea($this,"Container"));
		?>
		</div>
		<?php
	}
	
	protected function renderContainer(TWidgetArea $area){
		$items = $this->Items();
		foreach ($items as $mi){
			$this->RenderArea($this->GetArea($mi,"Item",$area));
		}
	}
	
	protected function areaCssClass(TWidgetArea $area){
		if ($area->Context() instanceof TAreasMenu)
			if ($area->Skin() == "Container") 
				return "container";
		if ($area->Context() instanceof TMenuItem)
			return "menu-item";
		return null;
	}	
	
	protected function renderItem(TWidgetArea $area){
		$context = $area->Context();
		$available = true;
		if ($context instanceof TActionMenuItem){
			$available = $context->Action()->Available();
			$calllink = $context->Action()->CallLink();
		?><a id="mil_<?php echo $context->ItemId(); ?>" class="menu-item-caption<?php echo $available?"":" disabled"; ?>" href="<?php echo $available?'javascript:jQuery.areas_AjaxCall(&quot;'.$calllink.'&quot;);':''; ?>"><?php __i($context); ?></a><?php	
		}
		else {
			if ($context instanceof TMenuItem)
				$available = $context->Available();
			if ($available){ 
				if ($context instanceof INavigable){
					?><a id="mil_<?php echo $context->ItemId(); ?>" href="<?php echo $context->Link(); ?>" class="menu-item-caption"><?php __i($context); ?></a><?php			
				}
				else {
					?><span id="mil_<?php echo $context->ItemId(); ?>" class="menu-item-caption"><?php  __i($context); ?></span><?php	
				}
			}
		}
		if ($available){
			$subs = $context->SubItems();
			if (count($subs) > 0){
				?>
				<div id="<?php echo $area->Id(); ?>_sm" class="areas-submenu" style="position:absolute;display:none;">
				<?php
			}
			foreach ($subs as $sl)
				$this->RenderArea($this->GetArea($sl,"Item"));
			if (count($subs) > 0){				
				?></div><?php
			}
		}
		$area->SkipWrapper = !$available;
	}
		
	protected function javaScript(){
		if (!self::$_menu_script_loaded_){
		?>
(function($){
	$.fn.extend({
			areasMenu:function(options){
				this.mousemove(function(e){return false;});
				var f = function(e){
						var menu = $(this).closest("div.areas-menu");
						var dd = $("div.areas-submenu",menu).not($(this).parents("div.areas-submenu").add($(this.parentElement).children("div.areas-submenu")));
						dd.hide("fast");
						var pp = $(this).parents("div.areas-submenu");
						pp.css("display","block");
						if ($('#'+this.id+'_sm').length > 0){
							if ($('#'+this.id+'_sm').get(0).style.display != "block"){
								var offset = $(this).position();
								var	t = 0,l = 0,zi = 0;
								var dsp = $(this).css("display");
								switch (dsp){
									case "block":{
										t = offset.top;
										l = offset.left+$(this).outerWidth();
									}break;
									case "inline":{
										t = offset.top+$(this).outerHeight();
										l = offset.left;
									}break;
								}
								zi = $(this).css("zIndex")+10;
								$('#'+this.id+'_sm').css("left",l).css("top",t).css("zIndex",zi).show("fast");
								$(document.body).one("mousemove",function(e){
									$("div.areas-submenu").hide("fast");
								});
								$(document.body).one("click",function(e){
									$("div.areas-submenu").hide("fast");
								});
								return false;
							}
						}
					};						
				
				if (options && options.PopupByClick)
				$("div.menu-item",this).click(f).mousemove(function(e){return false});
				else		
				$("div.menu-item",this).mouseenter(f).mousemove(function(e){return false});
					
				$("div.areas-submenu",this).bind("mousemove",function(){return false;});
			}
	});
})(jQuery);		
		<?php
			self::$_menu_script_loaded_ = true;
		}
		?>
		jQuery(document).one("ready",function(e){jQuery("#<?php echo $this->Name(); ?>").areasMenu(<?php if ($this->PopupByClick) echo "{PopupByClick:true}"; ?>);})
		<?php
	}
}
?>