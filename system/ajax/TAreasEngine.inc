<?php
TApplication::import("common::ajax");

interface IArea {
/**
 * @return string
 */	
	public function Id();	
	public function Render();
/**
 * @return IArea
 */
	public function ParentArea();
}

/**
 * @package Common
 * @subpackage Ajax
 * @category System
 * class that provides methods for outputing data stored in TRefreshItem. 
 */
class TAreasEngine extends TAjaxEngine {
	private $_queue_ = array();

	const AREA_ADD = "add";
	const AREA_REFERSH = "refresh";
	const AREA_DELETE = "delete";
	const AREA_MOVE = "move";
	 
/**
 * @ignore
 */	
	protected function raXML($actiontype,$area){
		$id = $area;
		if ($area instanceof IArea)
			$id = $area->Id();
		
		switch ($actiontype){
			case self::AREA_ADD:if ($area instanceof IArea){
				echo '<'.$actiontype.' id="'.$id.'" parent="'.$area->ParentArea()->Id().'"><![CDATA[';
				$area->Render();
				echo ']]></'.$actiontype.'>';				
			}break;
			case self::AREA_REFERSH:if ($area instanceof IArea){
				echo '<'.$actiontype.' id="'.$id.'"><![CDATA[';
				$area->Render();
				echo ']]></'.$actiontype.'>';
			}break;
			case self::AREA_DELETE:{
				echo '<'.$actiontype.' id="'.$id.'" />';				
			}break;
			case self::AREA_MOVE:{
				echo '<'.$actiontype.' id="'.$id.'" parent="'.$area->ParentArea()->Id().'" />';
			}break;
		}
	}
	
/**
 * @ignore
 */	
	protected function raJSON($actiontype,$area){
		switch ($actiontype){
			case self::AREA_ADD:if ($area instanceof IArea){
				TOutputBuffer::StartRecording();
				$area->Render();
				$this->AddJsonItem("a".$area->Id(),array($area->Id(),$actiontype,TOutputBuffer::StopRecording(),$area->ParentArea()->Id()));
			}break;
			case self::AREA_REFERSH:if ($area instanceof IArea) {
				TOutputBuffer::StartRecording();
				$area->Render();
				$this->AddJsonItem("r".$area->Id(),array($area->Id(),$actiontype,TOutputBuffer::StopRecording()));
			}break;
			case self::AREA_DELETE:{
				$this->AddJsonItem("d".(($area instanceof IArea)?$area->Id():$area),array((($area instanceof IArea)?$area->Id():$area),$actiontype,""));
			}break;
			case self::AREA_MOVE:if ($area instanceof IArea){
				$this->AddJsonItem("m".$area->Id(),array($area->Id(),$actiontype,"",$area->ParentArea()->Id()));
			}break;
		}
	}
/**
 * @ignore
 */	
	protected function raText($actiontype,$area){
		$id = $area;
		if ($area instanceof IArea)
			$id = $area->Id();
		echo "<--area--{".$id."}--{";
		if ($area instanceof IArea)
				echo $area->ParentArea()->Id();
		echo "}--{";
		echo $actiontype;	
		echo "}-->";
		if (($actiontype != self::AREA_DELETE) && ($actiontype != self::AREA_MOVE))
			if ($area instanceof IArea)
				$area->Render();		
	}

/**
 * Sends refresh data for specified widget according to response ajax content type.
 * @param TResponse $response
 * @param IAreaRenderer $renderer
 * @param string $areaname
 * @param mixed $data
 * @see IAreaRenderer
 */	
	public function RefreshArea(IArea $area){
		$this->_queue_[] = array(self::AREA_REFERSH,$area);
	}
	
	public function AddArea(IArea $area){
		$this->_queue_[] = array(self::AREA_ADD,$area);
	}
	
	public function DeleteArea($id){
		$this->_queue_[] = array(self::AREA_DELETE,$id);
	}
	
	public function MoveArea(IArea $area){
		$this->_queue_[] = array(self::AREA_MOVE,$area);
	}
	
	public function CommitResponse(TResponse $response){
		foreach ($this->_queue_ as $resp)
			switch ($response->CurrentContentType()){
				case TResponse::TYPE_XML:$this->raXML($resp[0],$resp[1]);break;
				case TResponse::TYPE_JSON:$this->raJSON($resp[0],$resp[1]);break;
				case TResponse::TYPE_TEXT:$this->raText($resp[0],$resp[1]);break;
			}
		parent::CommitResponse($response);	
	}
} 
?>