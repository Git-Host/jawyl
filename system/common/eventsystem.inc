<?php
/**
 * @package Common
 * @subpackage Events
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @package Common
 * @subpackage Events
 * @category System
 * basic class for implementing events
 * @property IEventSender $Sender
 * @property string $Name
*/
	class TEvent {
		private $_sender_;
		private $_name_;
		
		private static $_chains_ = array();
		private $_chain_; 
		
		public $StopHandling = false;
/**
 * constructor. SENDER attribute is set to $publisher
 * @param string $name event name
 * @param IEventPublisher $publisher event publisher
 */		
		public function __construct($name,IEventSender $sender){
			self::$_chains_[] = array();
			$this->_chain_ = count(self::$_chains_) - 1;
			$this->_name_ = $name;
			$this->_sender_ = $sender;
			self::$_chains_[$this->_chain_] = array($sender);
			$this->_sender_->Invoke($this);
		}
		
		public function __get($nm){
			switch ($nm){
				case "Name":return $this->_name_;break;
				case "Sender":return $this->_sender_;break;
			}
		}
		
		public function ReSend(IEventSender $sender){
			if (in_array($sender,self::$_chains_[$this->_chain_])) return;
			$e = clone $this;
			$e->_sender_ = $sender;
			self::$_chains_[$this->_chain_][] = $sender;
			$sender->Invoke($e);
		}
	}
	
	interface IEventHandler {
/** 
 * Returns event handling public methods available for delegation
 * @return callback[]
 */		
		public function EventHandlingDelegates(IEventListener $listener, TEvent $event);
	}

/** 
 * @property IEventHandler $DelegationProvider 
 */	
	interface IEventListener {
		public function Dispatch(TEvent $event);
	}
	
	
	interface IEventBroadCaster {
		public function BroadCastEvent(TEvent $event);
		public function AddEventListener(IEventListener $listener);
	}
	
/**
 * @author dakrasilnikov
 * @property IEventBroadCaster $BroadCasters
 */	
	interface IEventSender {
		public function AddBroadCaster(IEventBroadCaster $broadcaster);
/**
 * invokes an event
 */		
		public function Invoke(TEvent $event);
	}