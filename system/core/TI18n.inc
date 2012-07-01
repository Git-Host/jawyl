<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage I18n
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/


/**
 * format number according to current application locale
 *
 * @param number $number
 * @return string
 */

	function _n($number){
		return TI18n::Locale()->FormatNumber($number);
	}

/**
 * format number according to current application locale and send it ot output
 *
 * @param number $number
 * @uses _n
 */
	function __n($number){echo _n($number);}
	
/**
 * format date according to current application locale
 *
 * @param int|TDate $date
 * @return string
 */
	function _d($date){
		return TI18n::Locale()->FormatDate($date);
	}
	
/**
 * format date according to current application locale and send it ot output
 *
 * @param int|TDate $date
 * @uses _d
 */
	function __d($date){echo _d($date);}
	
/**
 * format time according to current application locale
 *
 * @param int $timestamp
 * @return string
 */
	function _t($timestamp){
		return TI18n::Locale()->FormatTime($timestamp);
	}
	
/**
 * format time according to current application locale and send it ot output
 *
 * @param int $timestamp
 * @uses _t
 */
	function __t($timestamp){echo _t($timestamp);}

/**
 * format datetime according to current application locale
 *
 * @param int|TDate $timestamp
 * @return string
 */
	function _dt($timestamp){
		return TI18n::Locale()->FormatDateTime($timestamp);
	}

/**
 * format datetime according to current application locale and send it ot output
 *
 * @param int|TDate $timestamp
 * * @uses _dt
 */
	function __dt($timestamp){echo _dt($timestamp);}

/**
 * format currency according to current application locale
 *
 * @param number $number
 * @return string
 */
	function _c($number){
		return TI18n::Locale()->FormatCurrency($number);
	}

/**
 * format currency according to current application locale and send it ot output
 *
 * @param number $number
 * @uses _c
 */
	function __c($number){echo _c($number);}

/**
 * translate text according to current application language
 *
 * @param string $text text to translate
 * @param string $domain text domain name, if null default domain is used
 * @param int $plural plural form number
 * @return string
 * @see ITranslator
 */
	function _i($text,$domain = null, $plural = null){
		return TI18n::Translate($domain,$text."",$plural);
	}

/**
 * translate text according to current application language and send to output
 *
 * @param string $text text to translate
 * @param string $domain text domain name, if null default domain is used
 * @param int $plural plural form number
 * @see ITranslator
 * @uses _i
 */
	function __i($text,$domain = null, $plural = null){echo _i($text,$domain,$plural);}

/**
 * Interface for implementing locales 
 * @package CoreRuntimeClasses
 * @subpackage I18n
 * @category Core
 */
	interface ILocale {
		const NT_NORMAL = 0;
		const NT_SHORT = 1;
		const NT_MIN = 2;
		
/**
 * gets locale description
 * @return string 
 */		
		public function Description();
/**
 * gets locale special symbols array
 * @return array 
 */		
		public function Symbols();
/**
 * gets locale date format
 * @return string 
 */		
		public function DateFormat();
/**
 * gets locale datetime format
 * @return string 
 */		
		public function DateTimeFormat();
/**
 * gets locale number format
 * @return string 
 */		
		public function NumberFormat();
/**
 * gets locale currency format
 * @return string 
 */		
		public function CurrencyFormat();
/**
 * formats number according to locale number format
 * @param number $number number value
 * @return string 
 */		
		public function FormatNumber($number);
/**
 * formats date according to locale date format
 * @param int|TDate $date datetime value
 * @return string 
 */		
		public function FormatDate($date);
/**
 * formats time according to locale time format
 * @param int|TDate $timestamp datetime value
 * @return string 
 */		
		public function FormatTime($timestamp);
/**
 * formats datetime according to locale datetime format
 * @param int|TDate $timestamp datetime value
 * @return string 
 */		
		public function FormatDateTime($timestamp);
/**
 * formats currency according to locale currency format
 * @param number $number currency value
 * @return string 
 */		
		public function FormatCurrency($number);
/**
 * set locale object language (select current locale settings).
 * @param string $language language name (particular language name implementation should be done in descendants)
 */		
		public function SetLanguage($language);
		
		public function MonthNames($type = ILocale::NT_NORMAL);
		
		public function WeekDayNames($type = ILocale::NT_NORMAL);
		
		public function FirstWeekDay();
	}
	
/**
 * @package CoreRuntimeClasses
 * @subpackage I18n
 * @category Core
 * Abstract locale class. 
 * Derive your custom locale classes from it.  
 * @uses ILocale
 * @uses TConfigurable
 */	
	abstract class TLocale extends TConfigurable implements ILocale {
		public function __construct($name, IConfigurable $container = null){
			parent::__construct($name,$container);
		}
	}
	
/**
 * @package CoreRuntimeClasses
 * @subpackage I18n
 * @category Core
 * Dummy locale class. 
 * Used to set default locale value when no locale specified. Has no formats and does no formatting (dates a formated with ISO 8601 format).  
 * @uses ILocale
 */	
	class TLocaleProxy implements ILocale {
/**
 * @ignore
 */		
		public function Name(){return "Locale proxy";}
/**
 * @ignore
 */		
		public function Description(){return "";}
/**
 * @ignore
 */		
		public function Symbols(){return array();}
/**
 * @ignore
 */		
		public function DateFormat(){return "C";}
/**
 * @ignore
 */		
		public function DateTimeFormat(){return "C";}
/**
 * @ignore
 */		
		public function NumberFormat(){return "";}
/**
 * @ignore
 */		
		public function CurrencyFormat(){return "";}
/**
 * @ignore
 */		
		public function FormatNumber($number){return $number;}
/**
 * @ignore
 */		
		public function FormatDate($timestamp){return date("C",$timestamp);}
/**
 * @ignore
 */		
		public function FormatTime($timestamp){return date("C",$timestamp);}
/**
 * @ignore
 */		
		public function FormatDateTime($timestamp){return date("C",$timestamp);}
/**
 * @ignore
 */		
		public function FormatCurrency($number){return $number;}
/**
 * @ignore
 */		
		public function SetLanguage($language){}
		
		public function MonthNames($type = ILocale::NT_NORMAL){return array();}
		
		public function WeekDayNames($type = ILocale::NT_NORMAL){return array();}
		
		public function FirstWeekDay(){return 0;}
	}
	
/**
 * Interface for implementing translators 
 * @package CoreRuntimeClasses
 * @subpackage I18n
 * @category Core
 * 
 */	
	interface ITranslator {
/**
 * translate text according to current translator language.
 * @param string $domain text domain name, to search for translation (particular implementation of text domains should be done in descendants)
 * @param string $text text to translate
 * @param int $plural plural form number
 * @return string  
 */		
		public function Translate($domain, $text, $plural = null);
/**
 * sets translator object current language, to use for translations
 * @param string $language language name (particular language name implementation should be done in descendants)
 */
		public function SetLanguage($language);
	}

/**
 * Abstract translator class. You can derive your custom translators from it.    
 * @package CoreRuntimeClasses
 * @subpackage I18n
 * @category Core
 * @uses TConfigurable
 * @uses ITranslator
 */	
	abstract class TTranslator extends TConfigurable implements ITranslator {
/**
 * @param string $name
 */
		public function __construct($name, IConfigurable $container = null){
			parent::__construct($name,$container);
		}	
	}
	
/**
 * Dummy translator class. Used for default setup when no translator specified. Does no translation.    
 * @package CoreRuntimeClasses
 * @subpackage I18n
 * @category Core
 * @uses ITranslator
 */	
	class TTranslatorProxy implements ITranslator {
/**
 * @ignore
 */		
		public function Translate($domain,$text, $plural = null){
			return $text;
		}
/**
 * @ignore
 */		
		public function SetLanguage($language){}
	}
	
/**
 * Static I18n class. Implement static access to current locale and translator methods.    
 * @package CoreRuntimeClasses
 * @subpackage I18n
 * @category Core
 * @see ILocale
 * @see ITranslator
 */	
	final class TI18n {
		private static $_language_;
		private static $_locale_;
		private static $_translator_;

/**
 * sets current application locale object by its name
 * @param string|ILocale $locale locale object or name
 */		
		public static function  SetLocale($locale){
			if (is_string($locale))
				self::$_locale_ = TApplication::$Application->Instance($locale);
			else if ($locale instanceof ILocale)	
				self::$_locale_ = $locale;
			else throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
			self::locale_obj()->SetLanguage(self::$_language_);
		}
		
/**
 * sets current application translator object by its name
 * @param string|ITranslator $translator translator object or name
 */		
		public static function SetTranslator($translator){
			if (is_string($translator))
				self::$_translator_ = TApplication::$Application->Instance($translator);
			else if ($translator instanceof ITranslator)
				self::$_translator_ = $translator;
			else throw new TCoreException(TCoreException::ERR_BAD_VALUE);				
			self::translator()->SetLanguage(self::$_language_);
		}

/**
 * sets current application language
 * @param string $language language name
 */		
		public static function SetLanguage($language){
			self::$_language_ = $language;
			self::locale_obj()->SetLanguage(self::$_language_);
			self::translator()->SetLanguage(self::$_language_);
		} 
		
/**
 * gets current application locale object
 * @return ILocale
 */		
		public static function Locale(){
			return self::locale_obj();
		}
		
/**
 * translates a text uses current application translator
 * @see ITranslator::Translate()
 */		
		public static function Translate($domain, $text, $plural = null){
			return self::translator()->Translate($domain,$text,$plural);
		}
		
		private static function translator(){
			if (!self::$_translator_)
				self::$_translator_ = new TTranslatorProxy();
			return self::$_translator_;
		}
		private static function locale_obj(){
			if (!self::$_locale_)
				self::$_locale_ = new TLocaleProxy();
			return self::$_locale_;
		}
	}
?>