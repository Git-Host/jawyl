<?php
/**
 * @package Common
 * @subpackage MvpEvents
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
require_once 'common/eventsystem.inc';
/**
 * @ignore
 */
require_once 'common/mvp.inc';


/**
 * @package Common
 * @subpackage MvpEvents
 * @category System
 * basic class for all mvp events.
*/
class TMvpEvent extends TEvent {
	const RELOAD_EVENT_NAME = "reload";
	const ACTION_EVENT_NAME = "action";
	const ACTIONPARAM_EVENT_NAME = "actionparam";
	const ACTIONSTATE_EVENT_NAME = "actionstate";
}

class TActionCallEvent extends TMvpEvent {
	public $ActionName;
	public function __construct($action,IEventSender $sender){
		$this->ActionName = $action;
		parent::__construct(self::ACTION_EVENT_NAME,$sender);
	}
}

class TActionParametersEvent extends TMvpEvent {
	public $Action;
	public function __construct(IAction $action,IEventSender $sender){
		$this->Action = $action;
		parent::__construct(self::ACTIONPARAM_EVENT_NAME,$sender);
	}
}

class TActionStateEvent extends TMvpEvent {
	public $Action;
	public function __construct(IAction $action,IEventSender $sender){
		$this->Action = $action;
		parent::__construct(self::ACTIONSTATE_EVENT_NAME,$sender);
	}
}

/**
 * @package Common
 * @subpackage MvpEvents
 * @category System
 * data reload event
*/
class TDataReloadEvent extends TMvpEvent {
/**
 * constructor
 * @see TEventPublisher
 */	
	public function __construct(IEventSender $sender){
		parent::__construct(self::RELOAD_EVENT_NAME,$sender);
	}
}