<?php
/*
 * @todo integrare with Iterator and IteratorAggregate interfaces
 */

/**
 * @package Common
 * @subpackage Iterator
 * @category System
 * interface for implementing iterators
 */	
	interface IIterator {
		const DT_RAW = 0;
		const DT_INSTANCE = 1;
		const DT_CLASS = 2;
		const DT_ATTRDEFINITION = 3;
		const DT_OBJECT = 4;
		const DT_MIXED = 5;

/**
 * fetch next item, returns true on success, otherwise false
 * @return boolean
 */		
		public function Next();
/**
 * gets current item
 * @return mixed
 */		
		public function Item();
/**
 * gets total item count
 * @return int
 */		
		public function ItemCount();
/**
 * gets iterator data type, must be one of IIterator constants
 * @return int
 */		
		public function DataType();
/**
 * moves by specified items count from current position and fetches subsequent item 
 * @param int $count
 * @return boolean
 */	
		public function Seek($count);
	}
	
/**
 * @package Common
 * @subpackage Orm
 * @category System
 * abstract iterator class
 * @property-read mixed $Item current item
 * current item properties can also be get as properties of iterator object
 */	
	abstract class TIterator implements IIterator {
		public function __get($nm){
			switch ($nm) {
				case "Item":return $this->Item();break;
				default:return $this->Item()->__get($nm);break;
			}
		}
	}
	
/**
 * @package Common
 * @subpackage Orm
 * @category System
 * iterator through an array
 */	
	class TArrayIterator extends TIterator {
/**
 * @var array array to iterate through
 */		
		protected $array;
/**
 * @var array array of values to iterate through
 */		
		protected $values;
/**
 * @var array array of keys
 */		
		protected $keys;
/**
 * @var int current iterator position
 */		
		protected $position;
		
/**
 * constructor
 */		
		public function __construct(array &$array){
			$this->array = $array;
			$this->values = array_values($array);
			$this->keys = array_keys($array);
			$this->position = -1;
		}
		
		public function Value($position){
			if (($position >= 0) && ($position < count($this->values)))
				return $this->values[$position];
			return null;
		}
		
		public function ValueByKey($key){
			if (key_exists($key,$this->array))
				return $this->array[$key];
			return null;
		}
		
/**
 * gets current item key
 * @return mixed
 */		
		public function Key($position = null){
			if (!is_null($position))
				if (($position >= 0) && ($position < count($this->keys)))
					return $this->keys[$position];
			if ($this->position < count($this->keys))
				return $this->keys[$this->position];
			return null;
		}
/**
 * @see IIterator::Next()
 */		
		public function Next(){
			$this->position = $this->position + 1;
			return $this->position < count($this->array);
		}
		
		public function Seek($count){
			$this->position = $this->position + $count;
			return $this->position < count($this->array);
		}
/**
 * @see IIterator::Item()
 */		
		public function Item(){
			if ($this->position < count($this->values)){
				return $this->values[$this->position];
			}
			return null;
		}
/**
 * @see IIterator::ItemCount()
 */		
		public function ItemCount(){
			return count($this->values);
		}
		
/**
 * resets iterator position to the beginning of array
 */		
		public function Reset(){
			$this->position = -1;
		}

/**
 * gets iterator type, for TArrayIterator always returns DT_RAW
 * @return int
 * @see IIterator::DataType()
 */		
		public function DataType(){return IIterator::DT_RAW;}
	}
	
	/**
 * @package Common
 * @subpackage Orm
 * @category System 
 * abstract iterator wrapper. Can be used to implement custom iterators that wrap other iterators.
 * This can be done by implementing IIterator::Item() method.
 * @see IIterator  
 */	
	abstract class TIteratorAdapter extends TIterator {
/**
 * @var IIterator
 */		
		protected $iterator;
/**
 * constructor
 */		
		public function __construct(IIterator $base){
			$this->iterator = $base;
		}
/**
 * @see IIterator::Next()
 */		
		public function Next(){return $this->iterator->Next();}
/**
 * @see IIterator::ItemCount()
 */		
		public function ItemCount(){return $this->iterator->ItemCount();}
/**
 * @see IIterator::DataType()
 */		
		public function DataType(){return $this->iterator->DataType();}	

		public function Seek($count){return $this->iterator->Seek();}
	}	
	
	abstract class TPagedIterator extends TIteratorAdapter {
		private $_pagesize_ = null;
		private $_page_ = null;
		private $_counter_ = 0;
		
		public function __construct(IIterator $base,$page = null,$pagesize = null){
			$this->_page_ = $page;
			$this->_pagesize_ = $pagesize;
			parent::__construct($base);
			if ($this->_pagesize_)
			 if ($this->_page_)
			 	$this->Seek($this->_page_*$this->_pagesize_);
		}		
		
		public function Next(){
			$result = parent::Next();
			$this->_counter_++;
			if ($result) $result = $this->_counter_ <= $this->_pagesize_;
			return $result;
		}
	}
?>