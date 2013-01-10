<?php
/**
 * @package Common
 * @subpackage MvpEvents
 * @category System
 * item selection event
*/

require_once 'common/mvpEvents.inc';

class TPresenterEvent extends TMvpEvent {
	const SELECT_EVENT_NAME = "select";
	const CREATE_EVENT_NAME = "create";
	const ADD_EVENT_NAME = "add";
	const DELETE_EVENT_NAME = "delete";
	const REMOVE_EVENT_NAME = "remove";
	const EDIT_EVENT_NAME = "edit";
	const MOVE_EVENT_NAME = "move";
	const OPEN_EVENT_NAME = "open";
}


class TItemSelectEvent extends TPresenterEvent {
/**
 * @var mixed id of node being selected
 */	
	public $NodeId;
/**
 * @var mixed id of previous selected node  
 */	
	public $PreviousNodeId;
/**
 * constructor
 * @param mixed $nodeid id of node being selected
 * @param mixed $previousid id of previous selected node
 * @param TEventPublisher $publisher event publisher
 * @see TEventPublisher
 */	
	public function __construct($nodeid,$previousid,IEventSender $sender){
		$this->NodeId = $nodeid;
		if (strlen($this->NodeId) == 0) $this->NodeId = null;
		$this->PreviousNodeId = $previousid;
		if (strlen($this->PreviousNodeId) == 0) $this->PreviousNodeId = null;
		parent::__construct(self::SELECT_EVENT_NAME,$sender);
	}
}
	
/**
 * @package Common
 * @subpackage MvpEvents
 * @category System
 * item creation event
*/
class TItemCreateEvent extends TPresenterEvent {
/**
 * @var mixed created node id
 */	
	public $NodeId;
/**
 * @var mixed optional parent node id
 */	
	public $ParentId;
/**
 * constructor
 * @param mixed $nodeid created node id
 * @param mixed $parentid parent node id, if created node has no parent should be set to null
 * @param TEventPublisher $publisher 
 * @see TEventPublisher 
*/
	public function __construct($nodeid,$parentid,IEventSender $sender){
		$this->NodeId = $nodeid;
		$this->ParentId = $parentid;
		parent::__construct(self::CREATE_EVENT_NAME,$sender);
	}
}

class TItemAddEvent extends TPresenterEvent {
/**
 * @var mixed created node id
 */	
	public $NodeId;
/**
 * @var mixed optional parent node id
 */	
	public $ParentId;
/**
 * constructor
 * @param mixed $nodeid added node id
 * @param mixed $parentid parent node id, if added node has no parent should be set to null
 * @param TEventPublisher $publisher 
 * @see TEventPublisher 
*/
	public function __construct($nodeid,$parentid,IEventSender $sender){
		$this->NodeId = $nodeid;
		$this->ParentId = $parentid;
		parent::__construct(self::ADD_EVENT_NAME,$sender);
	}	
}


	
/**
 * @package Common
 * @subpackage MvpEvents
 * @category System
 * item deletion event
*/
class TItemDeleteEvent extends TPresenterEvent {
/**
 * @var id of deleted item
 */	
	public $NodeId;
/**
 * constructor
 * @param mixed $nodeid id of deleted item
 * @param TEventPublisher $publisher
 * @see TEventPublisher
 */	
	public function __construct($nodeid,IEventSender $sender){
		$this->NodeId = $nodeid;
		parent::__construct(self::DELETE_EVENT_NAME,$sender);
	}
}

/**
 * @package Common
 * @subpackage MvpEvents
 * @category System
 * item removing event
*/
class TItemRemoveEvent extends TPresenterEvent {
/**
 * @var id of deleted item
 */	
	public $NodeId;
/**
 * constructor
 * @param mixed $nodeid id of deleted item
 * @param TEventPublisher $publisher
 * @see TEventPublisher
 */	
	public function __construct($nodeid,IEventSender $sender){
		$this->NodeId = $nodeid;
		parent::__construct(self::REMOVE_EVENT_NAME,$sender);
	}
}
	
/**
 * @package Common
 * @subpackage MvpEvents
 * @category System
 * item edition event
*/
class TItemEditEvent  extends TPresenterEvent {
/**
 * @var mixed id of item being edited
 */	
	public $NodeId;
/**
 * @var mixed previous id of item being edited
 */	
	public $PrevNodeId;
/**
 * @var string name of item attribute being edited 
 */	
	public $Attribute;
/**
 * @var mixed value item attribute set to
 */	
	public $Value;
/**
 * constructor
 * @param string $nodeid
 * @param string $attribute
 * @param mixed $value
 * @param TEventPublisher $publisher
 * @param mixed $pnid optional previous id of node being edited. If not specified $nodeid is used
 */	
	public function __construct($nodeid,$attribute,$value, IEventSender $sender, $pnid = null){
		$this->NodeId = $nodeid;
		$this->PrevNodeId = $pnid;
		$this->Attribute = $attribute;
		$this->Value = $value;
		parent::__construct(self::EDIT_EVENT_NAME,$sender);
	}
}
	
/**
 * @package Common
 * @subpackage MvpEvents
 * @category System
 * item moving event
*/
class TItemMoveEvent extends TPresenterEvent {
/**
 * @var mixed id of moving item
 */	
	public $NodeId;
/**
 * @var mixed id of new item parent
 */	
	public $ParentId;
/**
 * constructor
 * @param mixed $nodeid
 * @param mixed $parentid
 * @param TEventPublisher $publisher
 * @see TEventPublisher
 */	
	public function __construct($nodeid, $parentid, IEventSender $sender){
		$this->NodeId = $nodeid;
		$this->ParentId = $parentid;
		parent::__construct(self::MOVE_EVENT_NAME,$sender);
	}
}

class TItemOpenEvent extends TPresenterEvent {
/**
 * @var mixed id of moving item
 */	
	public $OpenedId;	
	
/**
 * constructor
 * @param mixed $nodeid
 * @param mixed $parentid
 * @param TEventPublisher $publisher
 * @see TEventPublisher
 */	
	public function __construct($openid,IEventSender $sender){
		$this->OpenedId = $openid;
		parent::__construct(self::OPEN_EVENT_NAME,$sender);
	}	
}
?>