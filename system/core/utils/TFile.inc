<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage Utilities
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @package CoreRuntimeClasses
 * @subpackage Utilities
 * @category Core
 * File wrapper.
 * 
 * @todo implement links for aliased web-directories
 */
   class TFile {
/**
 * @var string file name
 */   	
   	public $Name;
/**
 * @var string file mime type
 */   	
   	public $MimeType;
/**
 * @var string local path of file
 */   	
   	protected $path;
/**
 * @var int file size
 */   	
   	protected $size;
   	
/**
 * constructor. fills file object attributes.
 * @param string $name file name
 * @param string $path file path
 * @param string $type optional mime type
 * @param int $size optional size
 */   	
   	public function __construct($name,$path,$type = null,$size = null){
   		$this->Name = $name;
   		$this->MimeType = $type;
   		if (!TFileSystem::IsAbsolute($path))
   			$path = TApplication::FileStoragePath().'/'.$path;
   		$this->path = $path;
   		$this->size = $size;
   	}

/**
 * gets file contents
 * @return string
 */   	
   	public function GetContents(){
   		return file_get_contents($this->path);
   	}
   	
/**
 * gets a path where the file is actualy stored
 * @return string
 */   	
   	public function GetStorePath(){
   		return $this->path;
   	}
   	
   	protected function formLink($path){
   		$path = preg_replace('/[\\/\\\\]/','|',$path);
   		$docroot = preg_replace('/[\\/\\\\]/','|',$_SERVER["DOCUMENT_ROOT"]);
   		$link = str_ireplace($docroot,"",$path);
   		$link = str_replace('|','/',$link);
   		return $link;
   	}

/**
 * gets url to file
 */   	
   	public function GetLink(){
   		return $this->formLink($this->GetStorePath());
   	}
   	
/**
 * file string representation. by default evaluates to storage path. 
 * @return string
 */   	
   	public function __toString(){
		return $this->GetStorePath();	
	}   	
   }
   
   class TImage extends TFile {
   		protected $thumbnail;
   		protected $imageType;
	
   		public function __construct($name,$path,$thumbnail = null,$type = null,$size = null){
   			parent::__construct($name, $path,$type,$size);
   			if ($thumbnail){
   				if (!TFileSystem::IsAbsolute($thumbnail))
   					$thumbnail = TApplication::FileStoragePath().'/'.$thumbnail;
   				$this->thumbnail = $thumbnail;
   			} else $this->thumbnail = $this->path;
   		}   

   		public function ImageType(){
   			if (!file_exists($this->path))
   				throw new TCoreException(TCoreException::ERR_FILE_MISSING);	
   			
   			if (!$this->imageType){
   				if (function_exists('exif_imagetype'))
   					$this->imageType = exif_imagetype($this->path);
   				else {
   					$im = getimagesize($this->path);	
   					$this->imageType = $im[2];
   				}
   			}
   			return $this->imageType;	
   		}
   		
   		public function TypeExtension(){
   			return image_type_to_extension($this->ImageType());
   		}
   		
		public function GetThumbnailLink(){
			return $this->formLink($this->thumbnail);
		}
   	
   		public function SaveAs($filename,$type,$width=null,$height=null){
   			if (!file_exists($this->path))
   				throw new TCoreException(TCoreException::ERR_FILE_MISSING,array('file'=>$this->path));
   			if (!TFileSystem::IsAbsolute($filename))
   				$filename = TApplication::FileStoragePath().'/'.$filename;		
   			if (file_exists($filename))
   				unlink($filename);
   			TFileSystem::ForceDir(dirname($filename));
   				
   			list($orig_width, $orig_height, $orig_type, $attr) = getimagesize($this->path);
   			if (!$width)
   				$width = $orig_width;
   			if (!$height)
   				$height = $orig_height;

   			if ($orig_width < $orig_height)
   				$width = ($height / $orig_height) * $orig_width;
   			else 
   				$height = ($width / $orig_width) * $orig_height;
   				
			$image_dest = imagecreatetruecolor($width, $height);
			switch ($orig_type){
				case IMAGETYPE_GIF:$image_src = imagecreatefromgif($this->path);break;
				case IMAGETYPE_PNG:$image_src = imagecreatefrompng($this->path);break;
				case IMAGETYPE_JPEG:$image_src = imagecreatefromjpeg($this->path);break;
			}
			imagecopyresampled($image_dest, $image_src, 0, 0, 0, 0, $width, $height, $orig_width, $orig_height);
   			switch ($type){
   				case IMAGETYPE_GIF: imagegif($image_dest,$filename);break; 
   				case IMAGETYPE_PNG: imagepng($image_dest,$filename);break;
   				default:imagejpeg($image_dest,$filename);break;	
   			}
   		}
   }
   
/**
 * @package CoreRuntimeClasses
 * @subpackage Utilities
 * @category Core
 * Http 
 * Http uploaded file wrapper.
 */   
   class TUploadedFile extends TFile {
   		private $_stored_ = false;
   		
/**
 * constructor
 * @param array $file php uploaded file structure
 */   		
   		public function __construct(array $file){
   			parent::__construct($file["name"],$file["tmp_name"],$file["type"],$file["size"]);
   		}

/**
 * saves file to file folder. 
 * First it tries to save to folder specified by application FileStoragePath setting.
 * If saving fails, then file is saved to 'files' directory in application root folder, which is created if not exists. 
 * @see TApplication 
 */   		
   		public function Store($filename = null){
   			if ($this->_stored_) return $this->path;
   			if ($filename){
   				if (!TFileSystem::IsAbsolute($filename))
   					$filename = TApplication::FileStoragePath().'/'.$filename;
   			}
   			if (!$filename){
   				$path = TApplication::FileStoragePath();
   				$ts = new TDate();
   				$pi = pathinfo($this->Name);
   				$filename = $path."/".str_ireplace(".".$pi["extension"],"",$pi["basename"])."[".$ts->TimeStamp()."]".($pi["extension"]?".".$pi["extension"]:"");
   			}
   			
   			TFileSystem::ForceDir(dirname($filename));
   			
   			if (move_uploaded_file($this->path,$filename)){
   				$this->path = $filename;
   				$this->_stored_ = true;
   			}
   			return $this->path;
   		}
/**
 * gets actual file storage path
 * @return string
 * @see TFile::getStorePath()
 */   		
   		public function GetStorePath(){
   			return $this->Store();
   		}
   }
   
/**
 * @package CoreRuntimeClasses
 * @subpackage Utilities
 * @category Core
 * Http 
 * XmlRpc uploaded file wrapper.
 */    
   
   class TXmlRpcUploadedFile extends TFile {
/**
 * constructor. Saves uploaded file like TUploadedFile class does.
 * @param array XmlRpc uploaded file structure
 * @see TUploadedFile::Store
 */   			
   		public function __construct($struct){
   			$path = TApplication::FileStoragePath()."/".$struct["name"];
   			file_put_contents($path,$struct["bits"]->scalar);
   			parent::__construct($struct["name"],$path,$struct["type"],filesize($path));
   		}
   }
   
/**
 * @package CoreRuntimeClasses
 * @subpackage Utilities
 * @category Core
 * Http 
 * Wrapper for files stored in database. 
 * Files are converted to xml documents, with necessary attributes. 
 * Text of that document is stored in database.
 */    
   class TDBStoredFile extends TFile {
   		private $_content_;
   		
/**
 * constructor.
 * @param string $content - file xml document. 
 */   		
   		public function __construct($content){
   			$xml = new DOMDocument();
   			$xml->loadXML($content);
   			$xpath = new DOMXPath($xml);
			$name = $xpath->query("/file/header/name");
			$mime = $xpath->query("/file/header/mime");
			$content = $xpath->query("/file/content");
   			$this->_content_ = $content->item(0)->nodeValue;
   			parent::__construct($name->item(0)->nodeValue,"",$mime->item(0)->nodeValue,strlen($this->_content_));
   		}

/**
 * gets file actual contents (not xml document contents)
 * @return string
 */   		
      	public function GetContents(){
   			return $this->_content_;
   		}   		

/**
 * gets file path which evaluates to "" for compatibility.
 * @return string
 */   		
   		public function GetStorePath(){
   			return $this->path;
   		}

/**
 * gets file xml document
 * @return string
 */   		
   		public function __toString(){
   			return '<file><header><name>'.$this->Name.'</name><mime>'.$this->MimeType.'</mime></header><content><![CDATA['.$this->GetContents().']]></content></file>';
		}   	
   }
?>