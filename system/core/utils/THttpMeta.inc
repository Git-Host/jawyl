<?php

/**
 * @property string $ContentType
 * @property string[] $Headers
 * 
 * @author Крас
 */
class THttpMeta {
	const TYPE_XML = "xml";
	const TYPE_JSON = "json";
	const TYPE_JS = "javascript";
	const TYPE_HTML = "html";
	const TYPE_CSS = "css";
	const TYPE_GIF = "gif";
	const TYPE_JPEG = "jpg";
	const TYPE_PNG = "png";
	const TYPE_BMP = "bmp";
	const TYPE_ICON = "icon";
	const TYPE_TEXT = "text";	
	
/**
 * @var string
 * stores response ContentType property value
 */	
	protected $contentType = self::TYPE_TEXT;
/**
 * @var array
 * stores response headers as an associative array
 */	
	protected $headers = array();

	public function __get($nm){
		switch ($nm){
			case "ContentType":return $this->contentType;break;
			case "Headers":return $this->headers;break;
		}
	}
	
/**
 * gets response content types as an array
 */	
	protected static function contentTypes(){
		return array(
			self::TYPE_XML,
			self::TYPE_JSON,
			self::TYPE_JS,
			self::TYPE_HTML,
			self::TYPE_CSS,
			self::TYPE_BMP,
			self::TYPE_GIF,
			self::TYPE_JPEG,
			self::TYPE_PNG,
			self::TYPE_ICON,
			self::TYPE_TEXT);
	} 
	
	
/**
 * @ignore
 */	
	public function __set($nm,$value){
		switch ($nm){
			case "ContentType":{
				if (!in_array($value,self::contentTypes()))
					throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				$this->contentType = $value;
			}break;
			case "Headers":{
				if (is_array($value))
					$this->headers = $value;
				else {	
					preg_match_all('/([^:]*):([^:\r\n]*)/',$value,$h);
					$this->headers = array_combine($h[1],$h[2]);
				}
			}break;
		}
	}
	
/**
 * sends response additional headers
 */
	public function SendHeaders(){
		if (!isset($this->headers['Content-Type']))
			header($this->ContentTypeHeader());
		foreach ($this->headers as $key=>$value)
			header($key.":".$value);			
	}
	
/**
 * returns content type header
 * @param string $contentType optional content type. When not specified current content type is used.
 * @return string
 */	
	public function ContentTypeHeader($contentType = null){
		if (is_null($contentType))
			$contentType = $this->contentType;
		switch ($contentType){
			case self::TYPE_XML:return "Content-Type:application/xml; charset=utf-8";break;
			case self::TYPE_JS:
			case self::TYPE_JSON:return "Content-Type:text/javascript; charset=utf-8";break;
			case self::TYPE_HTML:return "Content-Type:text/html; charset=utf-8";break;
			case self::TYPE_CSS:return "Content-Type:text/css; charset=utf-8";break;
			case self::TYPE_GIF:return "Content-Type:img/gif";break;
			case self::TYPE_PNG:return "Content-Type:img/png";break;
			case self::TYPE_JPEG:return "Content-Type:img/jpeg";break;
			case self::TYPE_BMP:return "Content-Type:img/bmp";break;
			case self::TYPE_ICON:return "Content-Type:img/x-icon";break;
			case self::TYPE_TEXT:return "Content-Type:text/plain; charset=utf-8";break;
		}
	}
}