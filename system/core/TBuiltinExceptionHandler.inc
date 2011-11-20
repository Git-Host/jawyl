<?php
	require_once 'TException.inc';
	
	class TBuiltinExceptionHandler implements IExceptionHandler {
		private function _exception_header(Exception $e){
					?>
					<style>
						table.trace {
							border-collapse:collapse;
							border-style:solid;
							border-color:black;
							border-width:1px;
							margin:10px;
						}
						
						table.trace th {
							font-weight:bold;
							background-color:red;
							font-size:11pt;
							border-style:solid;
							border-color:black;
							border-width:2px;
							padding:4px;
						}
						
						table.trace td {
							font-size:11pt;
							border-style:dashed;
							border-color:black;
							border-width:1px;
							padding:3px;
						}
						
						h3 {
							color:red;
						}
						
						div.message {
							color:black;
							font-size:12pt;
							border-style:dotted;
							border-width:2px;
							border-color:red;
							margin:5px;
							padding:5px;
						}
						
						div.code {
							color:gray;
							border-style:double;
							border-color:black;
							border-width:3px;
							padding:5px;
							font-family:Verdana;
						}
						
						div.code div.error-line {
							font-weight:bold;
							color:red;
						}
					</style>
					<h3>
					<?php
					$error = $e instanceof ErrorException;
					echo $error?'Error ':'Exception ';
					echo $e->getCode();
					echo $error?' occured ':' raised ';
					?>
					in file <?php echo $e->getFile(); ?> on line <?php echo $e->getLine(); ?>
					</h3>
					<div class="message">
					<?php echo $e->getMessage(); ?>
					</div>
					<?php
					$trace = $e->getTrace();
					?>
					<h4>Stack trace:</h4>
					<table class="trace">
					<tr><th>n</th><th>file</th><th>line</th><th>call</th></tr>
					<?php
					$n = count($trace);
					foreach ($trace as $call_level){
						?>
					<tr>
						<td><?php echo $n; ?></td>
						<td><?php echo $call_level['file']; ?></td>
						<td style="text-align: right;"><?php echo $call_level['line']; ?></td>
						<td><?php echo $call_level['class'].$call_level['type'].$call_level['function']; ?>(<?php
						$args = array(); 
						foreach ($call_level['args'] as $arg) $args[] = is_object($arg)?(get_class($arg)):(is_numeric($arg)?$arg:(is_null($arg)?'null':("'".$arg."'")));
						echo join(',', $args); 
						?>)</td>
					</tr>
						<?php	
						$n--;
					}
					?></table><?php
 
		}
		
/**
 * Exception handling method
 * @param Exception $e
 * @return boolean
 */		
		public function HandleException(Exception $e){
			switch (TApplication::$Environment){
				case TApplication::ENV_PRODUCTION:{
					define_syslog_variables();
					if (openlog('Jawyl',LOG_NDELAY | LOG_PERROR,LOG_DAEMON)){
						if ($e instanceof ErrorException)
							syslog(LOG_ERR, 'error '.$e->getCode().':'.$e->getMessage());
						else	
							syslog(LOG_INFO, 'exception '.$e->getCode().':'.$e->getMessage());
					}
				}break;
				case TApplication::ENV_TESTING:{
					$this->_exception_header($e);
				}break;
				case TApplication::ENV_DEVELOPING:{
					$this->_exception_header($e);
					$lines = file($e->getFile());
					if (!empty($lines)){
						?>
						<div class="code">
						<?php
							$i = ($e->getLine() > 10)?($e->getLine() - 10):0;
							$n = count($lines);
							$n = ($e->getLine() + 10 < $n)?$e->getLine() + 10:$n;
/*
 * @todo : preserve code tab formatting
 */							 
							while ($i <= $n) {
        						echo '<div'.(($i+1) == $e->getLine()?' class="error-line"':'').'>'.htmlentities($lines[$i],ENT_QUOTES,'UTF-8').'</div>';
        						$i++;
    						}
						?>
						</div>
						<?php
						fclose($handle); 
					}
				}break;
			}
			return true;
		}		
	}