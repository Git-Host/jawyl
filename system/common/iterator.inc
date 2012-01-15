<?php
/**
 * @package Common
 * @subpackage Iterator
 * @category System
 * interface for implementing iterators
 */	
	interface IIterator extends Iterator {
		const DT_RAW = 0;
		const DT_INSTANCE = 1;
		const DT_CLASS = 2;
		const DT_ATTRDEFINITION = 3;
		const DT_OBJECT = 4;
		const DT_MIXED = 5;

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
 * moves to specified position and fetches subsequent item 
 * @param int $position
 * @return boolean
 */	
		public function Seek($position);
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
		
    	public function Current() {
    		return $this->Item();
    	}

    	public function Valid() {
        	return (bool)$this->Current();
    	}
    	
    	public function Seek($position){
    		$this->Rewind();
    		$i = 0;
    		while (($i < $position - 1) && $this->Next()) $i++;
    		return $this->Next();	
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
 * @var ArrayObject array to iterate through
 */		
		protected $arrayObject;
		
		protected $array;
		
		protected $iterator;
/**
 * constructor
 */		
		public function __construct(array $array){
			$this->array = $array;
			$this->interator = new TArrayIterator($this->array);
		}
		
		public function Value($position){
			$temp = array_values($this->array);
			if (($position >= 0) && ($position < count($temp)))
				return $temp[$position];
			return null;
		}
		
		public function ValueByKey($key){
			if (isset($this->array[$key]))
				return $this->array[$key];
			return null;
		}
		
/**
 * gets current item key
 * @return mixed
 */		
		public function Key(){
			return $this->iterator->key();
		}
/**
 * @see Iterator::Next()
 */		
		public function Next(){
			return $this->iterator->next();
		}
		
/**
 * @param int $position
 * @see IIterator::Seek()
 */		
		public function Seek($position){
			return $this->iterator->seek($position);
		}
/**
 * @see IIterator::Item()
 */		
		public function Item(){
			return $this->iterator->current();
		}
/**
 * @see IIterator::ItemCount()
 */		
		public function ItemCount(){
			return count($this->array);
		}
		
    	public function Rewind() {
    		$this->iterator->rewind();
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
 * @see IIterator::ItemCount()
 */		
		public function ItemCount(){return $this->iterator->ItemCount();}
/**
 * @see IIterator::DataType()
 */		
		public function DataType(){return $this->iterator->DataType();}	

/**
 * @param int $position
 * @see IIterator::Seek()
 */		
		public function Seek($position){return $this->iterator->Seek($position);}

/**
 * @see Iterator::Next()
 */		
		public function Next(){return $this->iterator->Next();}
		
/**
 * @see Iterator::Rewind()
 */		
		public function Rewind() {return $this->iterator->Rewind();}

/**
 * @see Iterator::Item()
 */		
		public function Item() {return $this->iterator->Item();}

/**
 * @see Iterator::Key()
 */		
		public function Key() {return $this->iterator->Key();}

/**
 * @see Iterator::Valid()
 */		
		public function Valid() {return $this->iterator->Valid();}		
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