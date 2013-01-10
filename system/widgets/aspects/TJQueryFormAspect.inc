<?php
	require_once 'common/item.inc';
	require_once 'common/mvp.inc';
	require_once 'widgets/TJQueryWidget.inc';

	final class TFormFieldType {
		const BOOL = "bool";
		const DATE = "date";
		const DEC = "dec";
		const IMAGE = "img";
		const FILE = "file";
		const HTML = "html";
		const INT = "int";
		const MULTILINE = "txt";
		const PASSWORD = "pwd";
		const STRING = "str";
		const SELECTION = "sel";
		const SET = "set";
		
		private $_code_;
		
		public function __construct($code){
			if (!in_array($code,array(self::BOOL,self::DATE,self::DEC,self::IMAGE,self::FILE,self::HTML,self::INT,self::MULTILINE,self::PASSWORD,self::STRING,self::SELECTION,self::SET)))
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			$this->_code_ = $code; 
		}
		
		public function __toString(){
			return $this->_code_;
		}
	}
	
	final class TFormField {
		private $_type_;
		private $_value_;
		private $_selection_;
		private $_size_;
				
		public function __construct(TFormFieldType $type, $value, $selection = null, $size = null){
			$this->_type_ = $type;
			$this->_value_ = $value;
			$this->_selection_ = $selection;
			$this->_size_ = $size;
		}
		
		public function __toString(){return $this->_value_;}
		public function __get($nm){
			switch ($nm){
				case "Type":return $this->_type_;break;
				case "Value":return $this->_value_;break;
				case "List":return $this->_selection_;break;
				case "Size":return $this->_size_;break;
			}
		}
	}


	final class TJQueryFormAspect { 
		private static function _get_field($base,$size = null){
			if (($base instanceof IPropertyMeta)){
				$value = null;
				if ($base instanceof IProperty)
					$value = $base->Value();
				$selection = $base->Selection();
				if (count($selection) > 0) {
					if ($base->Nullable())
						$selection = array(""=>"") + $selection;
					switch ($base->Type()){
						case TItemPropertyType::PT_SET:return new TFormField(new TFormFieldType(TFormFieldType::SET),$value,$selection,$size);break; 
						default:return new TFormField(new TFormFieldType(TFormFieldType::SELECTION),$value,$selection);break;
					} 	
				}					
				switch ($base->Type()){
					case TItemPropertyType::PT_BOOL:return new TFormField(new TFormFieldType(TFormFieldType::BOOL),$value);break;
					case TItemPropertyType::PT_DATE:return new TFormField(new TFormFieldType(TFormFieldType::DATE),$value);break;
					case TItemPropertyType::PT_DEC:return new TFormField(new TFormFieldType(TFormFieldType::DEC),$value,array(),$size);break;
					case TItemPropertyType::PT_FILE:return new TFormField(new TFormFieldType(TFormFieldType::FILE),$value);break;
					case TItemPropertyType::PT_HTML:return new TFormField(new TFormFieldType(TFormFieldType::HTML),$value,array(),$size);break;
					case TItemPropertyType::PT_IMAGE:return new TFormField(new TFormFieldType(TFormFieldType::IMAGE),$value);break;
					case TItemPropertyType::PT_INT:return new TFormField(new TFormFieldType(TFormFieldType::INT),$value,array(),$size);break;
					case TItemPropertyType::PT_MULTILINE:return new TFormField(new TFormFieldType(TFormFieldType::MULTILINE),$value,array(),$size);break;
					case TItemPropertyType::PT_PASSWORD:return new TFormField(new TFormFieldType(TFormFieldType::PASSWORD),$value);break;
					case TItemPropertyType::PT_REFERENCE:return new TFormField(new TFormFieldType(TFormFieldType::SELECTION),$value,$selection);break;
					case TItemPropertyType::PT_STRING:return new TFormField(new TFormFieldType(TFormFieldType::STRING),$value,array(),$size);break;
				}
			} else if ($base instanceof IActionParameter){
				$value = $base->Value();
				$selection = $base->Selectors();
				if (count($selection) > 0){
					switch ($base->Type()){ 
						case TActionParameterType::TYPE_SET:return new TFormField(new TFormFieldType(TFormFieldType::SET),$value,$selection,$size);break;
						default:return new TFormField(new TFormFieldType(TFormFieldType::SELECTION),$value,$selection);break;
					}
				} 	
				switch ($base->Type()){
					case TActionParameterType::TYPE_BOOLEAN:return new TFormField(new TFormFieldType(TFormFieldType::BOOL),$value);break;
					case TActionParameterType::TYPE_DATE:return new TFormField(new TFormFieldType(TFormFieldType::DATE),$value);break;
					case TActionParameterType::TYPE_FILE:return new TFormField(new TFormFieldType(TFormFieldType::FILE),$value);break;
					case TActionParameterType::TYPE_HTML:return new TFormField(new TFormFieldType(TFormFieldType::HTML),$value,array(),$size);break;
					case TActionParameterType::TYPE_INTEGER:return new TFormField(new TFormFieldType(TFormFieldType::INT),$value,array(),$size);break;
					case TActionParameterType::TYPE_PASSWORD:return new TFormField(new TFormFieldType(TFormFieldType::PASSWORD),$value);break;
					case TActionParameterType::TYPE_PICTURE:return new TFormField(new TFormFieldType(TFormFieldType::IMAGE),$value);break;
					case TActionParameterType::TYPE_REAL:return new TFormField(new TFormFieldType(TFormFieldType::DEC),$value,array(),$size);break;
					case TActionParameterType::TYPE_SELECT:return new TFormField(new TFormFieldType(TFormFieldType::SELECTION),$value,$selection);break;
					case TActionParameterType::TYPE_STRING:return new TFormField(new TFormFieldType(TFormFieldType::STRING),$value,array(),$size);break;
					case TActionParameterType::TYPE_TEXT:return new TFormField(new TFormFieldType(TFormFieldType::MULTILINE),$value,array(),$size);break;
				}
			} else if (is_numeric($base))
				return new TFormField(new TFormFieldType(TFormFieldType::DEC),$base);				
			else if (is_string($base))
				return new TFormField(new TFormFieldType(TFormFieldType::STRING),$base);
			else if (is_array($base))
				return new TFormField(new TFormFieldType(TFormFieldType::SELECTION),$base[0],$base);
			else if (is_bool($base))
				return new TFormField(new TFormFieldType(TFormFieldType::BOOL),$base);
			else if (is_integer($base))
				return new TFormField(new TFormFieldType(TFormFieldType::INT),$base);
			else if (is_float($base))
				return new TFormField(new TFormFieldType(TFormFieldType::DEC),$base);
			return new TFormField(new TFormFieldType(TFormFieldType::STRING),$base);
		}

		public static function RenderField($id,$base,$name = null, $ajax_upload = false, IFilePropertyPublisher $filepublisher = null, $size = null, $tabindex = null){
			if (is_null($name)) $name = $id;
			$fld = self::_get_field($base,$size);
			switch ($fld->Type){
				case TFormFieldType::SELECTION:{
					$sel = $fld->List;
					?><select id="<?php echo $id; ?>" class="<?php echo TFormFieldType::SELECTION; ?>" name="<?php echo $name; ?>"<?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?>><?php 
					foreach ($sel as $key=>$value){
						?><option value="<?php echo $key; ?>"<?php echo (((string)$fld->Value == (string)$key)?' selected':''); ?>><?php __i($value); ?></option><?php
					}
					?></select><?php
				}break;
				case TFormFieldType::SET:{
					?><select id="<?php echo $id; ?>" class="<?php echo TFormFieldType::SET; ?>" name="<?php echo $name; ?>[]" multiple="multiple"<?php  echo $fld->Size?' size="'.$fld->Size.'"':''; ?><?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?>><?php
					$sel = $fld->List; 
					foreach ($sel as $key=>$value){
						?><option value="<?php echo $key; ?>"<?php echo (((string)$fld->Value == (string)$key)?' selected':''); ?>><?php __i($value); ?></option><?php
					}
					?></select><?php
				}break;
				case TFormFieldType::BOOL:{
					?><input id="<?php echo $id; ?>" type="checkbox" class="<?php echo TFormFieldType::BOOL; ?>" name="<?php echo $name; ?>" <?php echo ($fld->Value?'checked':''); ?><?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?>/><?php
				}break;
				case TFormFieldType::DATE:{
					?><input id="<?php echo $id; ?>" type="text" class="<?php echo TFormFieldType::DATE; ?>" name="<?php echo $name; ?>" value="<?php echo htmlentities($fld->Value); ?>"<?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?> /><?php
				}break;
				case TFormFieldType::DEC:{
					?><input id="<?php echo $id; ?>" type="text" class="<?php echo TFormFieldType::DEC; ?>" name="<?php echo $name; ?>" value="<?php echo $fld->Value; ?>"<?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?> /><?php
				}break;
				case TFormFieldType::IMAGE:{
					if ($ajax_upload){
						$f = $fld->Value;
						?><div id="<?php echo $id; ?>" class="image"><?php
			 			if (!is_null($f)){
			 				?><div><?php echo $f->Name; ?></div><div><?php if ($filepublisher) echo $filepublisher->FileLink($base); ?></div><?php
			 			}
						?></div><input type="hidden" name="ajaxcall" value="on" /><?php
					} else {
						?><input type="file" id="<?php echo $id; ?>" name="<?php echo $name; ?>" class="image"<?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?> /><?php
					}
				}break;
				case TFormFieldType::FILE:{
					if ($ajax_upload){
						$f = $fld->Value;
						?><div id="<?php echo $id; ?>" class="file"><?php
			 			if (!is_null($f)){
			 				?><div><?php echo $f->Name; ?></div><div><?php if ($filepublisher) echo $filepublisher->FileLink($base); ?></div><?php
			 			}
			 			?></div><input type="hidden" name="ajaxcall" value="on" /><?php
					} else {
						?><input type="file" id="<?php echo $id; ?>" name="<?php echo $name; ?>" value="" class="file"<?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?> /><?php
					}
				}break;
				case TFormFieldType::HTML:{
					?><textarea id="<?php echo $id; ?>" class="<?php echo TFormFieldType::HTML; ?>" name="<?php echo $name; ?>"<?php  echo $fld->Size?' rows="'.$fld->Size.'"':''; ?><?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?>><?php echo htmlentities($fld->Value,ENT_QUOTES,"UTF-8"); ?></textarea><?php
				}break;
				case TFormFieldType::INT:{
					?><input id="<?php echo $id; ?>" type="text" class="<?php echo TFormFieldType::STRING; ?>" name="<?php echo $name; ?>" value="<?php echo $fld->Value; ?>"<?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?> /><?php
				}break;
				case TFormFieldType::MULTILINE:{
					?><textarea id="<?php echo $id; ?>" class="<?php echo TFormFieldType::MULTILINE; ?>" name="<?php echo $name; ?>"<?php  echo $fld->Size?' rows="'.$fld->Size.'"':''; ?><?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?>><?php echo htmlentities($fld->Value,ENT_QUOTES,"UTF-8"); ?></textarea><?php
				}break;
				case TFormFieldType::PASSWORD:{
					?><input id="<?php echo $id; ?>" type="password" class="<?php echo TFormFieldType::PASSWORD; ?>" name="<?php echo $name; ?>" value=""<?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?> /><?php
				}break;
				case TFormFieldType::STRING:{
					?><input id="<?php echo $id; ?>" type="text" class="<?php echo TFormFieldType::STRING; ?>" name="<?php echo $name; ?>" value="<?php echo htmlentities("".$fld->Value,ENT_QUOTES,"UTF-8"); ?>"<?php echo !is_null($tabindex)?(' tabindex="'.$tabindex.'"'):''; ?> /><?php
				}break;
			}
		}
	}
?>