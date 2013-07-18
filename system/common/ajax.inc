<?php
/**
 * @package Common
 * @subpackage Ajax
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @package Common
 * @subpackage Ajax
 * @category System
 * Ajax engine interface specification
 * @see TRefreshItem 
 */
interface IAjaxEngine {
/**
 * Outputs error message formated according to response according content type
 * @param TResponse $response
 * @param string $msg message to output
 * @see TResponse 
 */	
	public function Error(TResponse $response, $msg);	
/**
 * Starts ajax output according to response ajax content type. Also sends necessary headers when needed.
 * @see TResponse
 * @see THttpRequest
 */	
	public function StartResponse(TResponse $response, THttpRequest $request);
/**
 * Commits ajax output according to response ajax content type.
 */	
	public function CommitResponse(TResponse $response);
} 

abstract class TAjaxEngine extends TConfigurable implements IAjaxEngine {
	protected $jsonItems = array();
	
/**
 * Outputs error message formated according to response content type
 * @param TResponse $response
 * @param string $msg message to output
 * @see TResponse 
 */	
	public function Error(TResponse $response, $msg){
		switch ($response->CurrentContentType()){
			case THttpMeta::TYPE_XML:echo '<error><![CDATA['.$msg.']]></error>';break;
			case THttpMeta::TYPE_JSON:$this->AddJsonItem("error",$msg);break;
			case THttpMeta::TYPE_TEXT:echo '[--error--]'.$msg.'[--error--]';break;
		}	
	}
	
/**
 * Starts ajax output according to response ajax content type. Also sends necessary headers when needed.
 * @see TResponse
 * @see THttpRequest
 */	
	public function StartResponse(TResponse $response, THttpRequest $request){
		/* FIX IE behaviour. 
			When loading via IFRAME IE treats javascript as file. 
			Done for ajax file uploads. */ 
		if (($response->CurrentContentType() != THttpMeta::TYPE_JSON) || $request->IsXmlHttpRequest)
				header($response->ContentTypeHeader($response->CurrentContentType()));
			else
				header("Content-Type:text/html; charset=utf-8");		
				
		switch ($response->CurrentContentType()){
			case THttpMeta::TYPE_XML:echo '<?xml version="1.0" encoding="UTF-8"?>'."\n".'<response>'."\n";break;
		}
		
	}
/**
 * Commits ajax output according to response ajax content type.
 */	
	public function CommitResponse(TResponse $response){
		switch ($response->CurrentContentType()){
			case THttpMeta::TYPE_XML:echo '</response>';break;
			case THttpMeta::TYPE_JSON:echo json_encode($this->jsonItems);break;
		}
	}

	protected function AddJsonItem($name,$value){
		$this->jsonItems[$name] = $value;
	}
}

?>