<?php
/**
 * @ignore 
 */
TApplication::import("core::utils::THttpMeta");
TApplication::import("common::xml");

/**
 * @package Common
 * @subpackage Soap
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

interface ISoapServer {
	public function Handle();
	public function WSDL(); 
}

/**
 * Soap options
 * 
 * @author dakrasilnikov
 *
 * @property string|int $SoapVersion
 * @property string $Login
 * @property string $Password
 * @property string $ProxyHost
 * @property string $ProxyPort
 * @property string $ProxyPassword
 * @property string $LocalCert
 * @property string $PassPhrase
 * @property string|int $Authentication
 * @property string|boolean $Compression
 * @property string|boolean $Trace
 * @property string|boolean $Exceptions
 * @property int $ConnectionTimeout
 * @property string $UserAgent
 * @property string|array $StreamContext
 * @property string|boolean $KeepAlive
 * @property string $Encoding
 * @property string|int $CacheWsdl
 * @property string|int $Feature
 */
final class TSoapOptions {
	const TYPE_SERVER = 0;
	const TYPE_CLIENT = 1;
	
	private $_soap_version_ = SOAP_1_2;
	private $_login_ = false;
	private $_password_ = false; 
	private $_proxy_host_ = false;
	private $_proxy_port_ = false;
	private $_proxy_login_ = false;
	private $_proxy_password_ = false;
	private $_local_cert_ = false;
	private $_passphrase_ = false;
	private $_authentication_ = SOAP_AUTHENTICATION_BASIC;
	private $_compression_ = false;
	private $_trace_ = false;
	private $_exceptions_ = true;
	private $_connection_timeout_ = false; 
	private $_user_agent_ = false;
	private $_stream_context_ = false;
	private $_keep_alive_ = false; 
	private $_encoding_ = false;
	
	private $_cache_wsdl_ = WSDL_CACHE_NONE;
	private $_features_ = 0; 

	private $_type_map_ = array();
	private $_class_map_ = array();

	public function ForConstructor($type = TSoapOptions::TYPE_CLIENT){
		$result = array(
			'soap_version' => $this->_soap_version_,
			'compression' => $this->_compression_, 
			'cache_wsdl' => $this->_cache_wsdl_
	
		);
		
		if ($this->_encoding_)
			$result['encoding'] = $this->_encoding_;
		if (!empty($this->_type_map_))
			$result['type_map'] = $this->_type_map_;
		if (!empty($this->_class_map_))
			$result['class_map'] = $this->_class_map_;
		if ($this->_features_)
			$result['features'] = $this->_features_;
	
		if ($type == self::TYPE_CLIENT){
			if ($this->_login_)
				$result['login'] = $this->_login_;
			if ($this->_password_)
				$result['password'] = $this->_password_;
			if ($this->_proxy_host_)
				$result['proxy_host'] = $this->_proxy_host_;
			if ($this->_proxy_port_)
				$result['proxy_port'] = $this->_proxy_port_;
			if ($this->_proxy_login_)
				$result['proxy_login'] = $this->_proxy_login_;
			if ($this->_proxy_password_)
				$result['proxy_password'] = $this->_proxy_password_;
			if ($this->_local_cert_)
				$result['local_cert'] = $this->_local_cert_;
			if ($this->_passphrase_)
				$result['passphrase'] = $this->_passphrase_;
			if ($this->_connection_timeout_)
				$result['connection_timeout'] = $this->_connection_timeout_;
			if ($this->_user_agent_)
				$result['user_agent'] = $this->_user_agent_;
			if (!empty($this->_stream_context_))
				$result['stream_context'] = $this->_stream_context_;

			$result = array_merge($result,array(
					'authentication' => $this->_authentication_,
					'trace' => $this->_trace_,
					'exceptions' => $this->_exceptions_,
					'user_agent' => $this->_user_agent_,
					'keep_alive' => $this->_keep_alive_ 
			));	
		}
		return $result;
	}
	
/**
 * @param string $wsdlname
 * @param string $classname
 */	
	public function MapClass($wsdlname,$classname){
		$this->_class_map_[$wsdlname] = $classname;
	}

/**
 * @param string $wsdltype
 * @param string $namespace
 * @param callback $fromXML
 * @param callback $toXML
 */	
	public function MapType($wsdltype,$namespace,$fromXML,$toXML){
		$this->_type_map_[] = array(
			'type_name' => $wsdltype,
			'type_ns' => $namespace,
			'from_xml' => $fromXML,
			'to_xml' => $toXML
		);
	}
	
	private function _check_constant($value, array $values){
		$value = is_string($value)?constant($value):$value;
		if (in_array($value,$values))
			return $value;
		return false;
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case 'SoapVersion':{
				switch ($value){
					case '1.1':$value = SOAP_1_1;break;
					case '1.2':$value = SOAP_1_2;break;
					default:{
						if (is_string($value)){
							if  (is_numeric($value))
								$value = ($value >= 2)?SOAP_1_2:SOAP_1_1;
							else 
								$value = constant($value);	
						}
						$value = $this->_check_constant($value, array(SOAP_1_1,SOAP_1_2)); 
					}break;
				}	
				if ($value !== false)	
					$this->_soap_version_ = $value;				
			}break;
			case 'Encoding':$this->_encoding_ = $value;break;
			case 'CacheWsdl':$this->_cache_wsdl_ = $this->_check_constant($value, array(WSDL_CACHE_NONE,WSDL_CACHE_BOTH,WSDL_CACHE_DISK,WSDL_CACHE_MEMORY));break;
			case 'Feature':{
				if (!$value) 
					$this->_features_ = 0;
				else 
					$this->_features_ = $this->features | $this->_check_constant($value, array(SOAP_WAIT_ONE_WAY_CALLS,SOAP_SINGLE_ELEMENT_ARRAYS, SOAP_USE_XSI_ARRAY_TYPE)); 	
			}break;
			case 'Login':$this->_login_ = $value;break;
			case 'Password':$this->_password_ = $value;break;
			case 'ProxyHost':$this->_proxy_host_ = $value;break;
			case 'ProxyPort':$this->_proxy_port_;break;
			case 'ProxyPassword':$this->_proxy_password_ = $value;break;
			case 'LocalCert':$this->_local_cert_ = $value;break;
			case 'PassPhrase':$this->_pass_phrase_ = $value;break;
			case 'Authentication':$this->_authentication_ = $this->_check_constant($value, array(SOAP_AUTHENTICATION_BASIC,SOAP_AUTHENTICATION_DIGEST));break;
			case 'Compression':$this->_compression_ = TConvertions::ConvertToBoolean($value);break;
			case 'Trace':$this->_trace_ = TConvertions::ConvertToBoolean($value);break;
			case 'Exceptions':$this->_exceptions_ = TConvertions::ConvertToBoolean($value);break;
			case 'ConnectionTimeout':$this->_connection_timeout_ = (int)$value;break;
			case 'UserAgent':$this->_user_agent_ = $value;break;
			case 'StreamContext':$this->_stream_context_ = TConvertions::ConvertToArray($value);break;
			case 'KeepAlive':$this->_keep_alive_ = TConvertions::ConvertToBoolean($value);break;
			case 'ClassMappings':{
				$mappings = $value;
				if (!is_array($mappings))
					$mappings = TConvertions::ConvertToArray($mappings,array(';','='),array(false,true));
				foreach ($mappings as $wsdlname=>$classname)
					$this->MapClass($wsdlname, $classname);
			}break;
		}
	}
	
	public function __get($nm){
		switch ($nm){
			case 'SoapVersion':return $this->_soap_version_;break;
			case 'Login':return $this->_login_;break;
		//	case 'Password':return $this->_password_;break;
			case 'ProxyHost':return $this->_proxy_host_;break;
			case 'ProxyPort':return $this->_proxy_port_;break;
		//	case 'ProxyPassword':return $this->_proxy_password_;break;
			case 'LocalCert':return $this->_local_cert_;break;
			case 'PassPhrase':return $this->_pass_phrase_;break;
			case 'Authentication':return $this->_authentication_;break;
			case 'Compression':return $this->_compression_;break;
			case 'Trace':return $this->_trace_;break;
			case 'Exceptions':return $this->_exceptions_;break;
			case 'ConnectionTimeout':return $this->_connection_timeout_;break;
			case 'UserAgent':return $this->_user_agent_;break;
			case 'StreamContext':return $this->_stream_context_;break;
			case 'KeepAlive':return $this->_keep_alive_;break;
			case 'Encoding':return $this->_encoding_;break;
			case 'CacheWsdl':return $this->_cache_wsdl_;break;
			case 'Feature':return $this->_features_;break;		
			
		}
	}
	
	
	public function IsOption($name){
		return in_array($name,array(
								'SoapVersion',
								'Login',
								'Password',
								'ProxyHost',
								'ProxyPort',
								'ProxyPassword',
								'LocalCert',
								'PassPhrase',
								'Authentication',
 								'Compression',
 								'Trace',
 								'Exceptions',
 								'ConnectionTimeout',
 								'UserAgent',
 								'StreamContext',
 								'KeepAlive',
 								'Encoding',
								'CacheWsdl',
 								'Feature',
								'ClassMappings'		
		));
	}
}

/**
 * 
 * Basic class for all soap servers
 * @author dakrasilnikov
 * @property string $Wsdl
 */
class TSoapServer extends TConfigurable implements ISoapServer {
	protected $wsdl = false;
/**
 * @var THttpMeta
 */	
	protected $httpMeta;
	
/**
 * @var TSoapOptions
 */	
	protected $options;
	
/**
 * @var IXSLTransformer
 */	
	protected $_ioc_xsl_pre_processor_;
	
/**
 * @var IXSLTransformer
 */	
	protected $_ioc_xsl_post_processor_;
	
	
	public function __construct($name, IConfigurable $container = null){
		parent::__construct($name,$container);
		$this->httpMeta = new THttpMeta();
		$this->httpMeta->ContentType = THttpMeta::TYPE_XML;
		$this->options = new TSoapOptions();
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case 'WSDL':$this->wsdl = TApplication::$Application->AdjustPath($value);break;
			case 'HttpHeaders':$this->httpMeta->Headers = $value;break;
			default:{
					if ($this->options->IsOption($nm))
						$this->options->$nm = $value;
					else 
						parent::__set($nm,$value);
			}break;
		}
		
	}
	
	protected function getHandler(){
		return $this;
	}
	
	public function Handle(){
		$server = new SoapServer($this->wsdl,$this->options->ForConstructor(TSoapOptions::TYPE_SERVER));
		$server->setObject($this->getHandler());
		$this->httpMeta->SendHeaders();
		
		try {
			if ($this->_ioc_xsl_pre_processor_ instanceof IXSLTransformer){
				global $HTTP_RAW_POST_DATA;
				if ($this->_ioc_xsl_post_processor_ instanceof IXSLTransformer)
					TOutputBuffer::StartRecording();
				$server->handle($this->_ioc_xsl_pre_processor_->TransformXML($HTTP_RAW_POST_DATA));
				if ($this->_ioc_xsl_post_processor_ instanceof IXSLTransformer)
					echo $this->_ioc_xsl_post_processor_->TransformXML(TOutputBuffer::StopRecording());
			} else
				$server->handle();
		} catch (Exception $e) {
			$server->fault($e->getCode(),$e->getMessage());
		}
		return true;
	}
		
	protected function compiledWsdlPath(){
/*
 * @todo Implement WSDL compilation
 **/		
		
	}
	
	public function WSDL(){
		$path = $this->wsdl;
		if (!$path)
			$path = $this->compiledWsdlPath();
		echo file_get_contents($path);
	} 
}
