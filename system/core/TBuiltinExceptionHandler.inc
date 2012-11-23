<?php
	require_once 'TException.inc';
	
	class TBuiltinExceptionHandler implements IExceptionHandler {
		private function _exception_header(Exception $e){
					?>
					<style>
						.error {
							font-family:Verdana;
						}
						.error table.trace {
							border-collapse:collapse;
							border-style:solid;
							border-color:black;
							border-width:2px;
							margin:10px;
						}
						
						.error table.trace th {
							font-weight:bold;
							background-color:orange;
							font-size:11pt;
							border-style:solid;
							border-color:black;
							border-width:2px;
							padding:4px;
							font-size:10pt;
						}
						
						.error table.trace td {
							font-size:11pt;
							border-style:dashed;
							border-color:black;
							border-width:1px;
							padding:3px;
							font-size:10pt;
						}
						
						.error h3 {
							color:red;
							font-size:12pt;
						}
						
						.error div.part-title {
							font-weight:bold;
							font-size:10pt;
							margin:3px;
						}
						
						.error div.message {
							color:black;
							font-size:12pt;
							border-style:dotted;
							border-width:2px;
							border-color:red;
							margin:5px;
							padding:5px;
							font-size:11pt;
							background-color:#ffeedd;
						}
						
						.error table.code {
							color:gray;
							border-style:double;
							border-color:black;
							border-width:3px;
							font-size:12pt;
							border-collapse:collapse;
							width:100%;
							overlow:hidden;
							table-layout: fixed;
						}
						
						.error table.code td {
							border-style:none;
							padding:2px;
							font-size:10pt;
						}
						
						.error table.code td.line {
							overflow:hidden;
							white-space: nowrap;
						}
						
						.error table.code td.line div.tab {
							width:30px;
							display:inline;
						}
						
						
						.error table.code tr.error-line {
							font-weight:bold;
							color:red;
						}
						
						.error span.syspath {
							font-weight:lighter;
							color:silver;
						}
					</style>
					<div class="error">
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
					<div class="part-title">Stack trace:</div>
					<table class="trace">
					<tr><th>n</th><th>call</th><th>line</th><th>file</th></tr>
					<?php
					$n = count($trace);
					foreach ($trace as $call){
						?>
					<tr>
						<td><?php echo $n; ?></td>
						<td><?php
						if (isset($call['class'])) 
							echo $call['class'];
						if (isset($call['type']))	
							echo $call['type'];
						if (isset($call['function']))
							echo $call['function']; 
						?>(<?php
						$args = array(); 
						if (isset($call['args']))
							foreach ($call['args'] as $arg) $args[] = is_object($arg)?(get_class($arg)):(is_numeric($arg)?$arg:(is_null($arg)?'null':("'".(is_array($arg)?'Array':$arg)."'")));
						echo join(',', $args); 
						?>)</td>
						<td style="text-align: right;"><?php if (isset($call['line'])) echo $call['line']; ?></td>
						<td><?php if (isset($call['file'])) {
								$file = str_replace('\\', '/', $call['file']);
								if (stristr($file,TApplication::$SystemPath))
									echo str_ireplace(TApplication::$SystemPath, '<span class="syspath">SYS_PATH</span>', $file);
								else if (stristr($file,TApplication::$Application->AppPath))
									echo str_ireplace(TApplication::$Application->AppPath, '<span class="syspath">APP_PATH</span>', $file);
								else 
									echo $file;	
						} ?></td>
					</tr>
						<?php	
						$n--;
					}
					?></table><?php
					die;
 
		}
		
	private function _console_exception_header(Exception $e){
		$error = $e instanceof ErrorException;
		echo $error?'Error ':'Exception ';
		echo $e->getCode();
		echo $error?' occured ':' raised ';
		?>in file <?php echo $e->getFile(); ?> on line <?php 
		echo $e->getLine();echo "\n"; 
		echo $e->getMessage();echo "\n";
		$trace = $e->getTrace();
		?>Stack trace:<?php echo "\n"; 
		$n = count($trace);
			foreach ($trace as $call){
				 echo $n.". ";
				if (isset($call['class'])) 
					echo $call['class'];
				if (isset($call['type']))	
					echo $call['type'];
				if (isset($call['function']))
					echo $call['function']; 
			?>(<?php
					$args = array(); 
						if (isset($call['args']))
							foreach ($call['args'] as $arg) $args[] = is_object($arg)?(get_class($arg)):(is_numeric($arg)?$arg:(is_null($arg)?'null':("'".$arg."'")));
						echo join(',', $args); 
				?>) on line <?php if (isset($call['line'])) echo $call['line']; ?> in <?php 
						if (isset($call['file'])) {
								$file = str_replace('\\', '/', $call['file']);
								if (stristr($file,TApplication::$SystemPath))
									echo str_ireplace(TApplication::$SystemPath, 'SYS_PATH', $file);
								else if (stristr($file,TApplication::$Application->AppPath))
									echo str_ireplace(TApplication::$Application->AppPath, 'APP_PATH', $file);
								else 
									echo $file;	
						}
						echo "\n"; 
						$n--;
					}
		}		
		
		
		
/**
 * Exception handling method
 * @param Exception $e
 * @return boolean
 */		
		public function HandleException(Exception $e){
			switch (TApplication::$Environment){
				case TApplication::ENV_PRODUCTION:{
					if (openlog('Jawyl',LOG_NDELAY | LOG_PERROR,LOG_DAEMON)){
						if ($e instanceof ErrorException)
							syslog(LOG_ERR, 'error '.$e->getCode().':'.$e->getMessage());
						else	
							syslog(LOG_INFO, 'exception '.$e->getCode().':'.$e->getMessage());
					}
				}break;
				case TApplication::ENV_TESTING:{
					if (TApplication::$Application->CallType == TApplication::CALL_CONSOLE){
						$this->_console_exception_header($e);
					} else {
						$this->_exception_header($e);
						?>
						</div>
						<?php
					}
				}break;
				case TApplication::ENV_DEVELOPING:{
					if (TApplication::$Application->CallType == TApplication::CALL_CONSOLE){
						$this->_console_exception_header($e);
					} else {
					$this->_exception_header($e);
/*
 * @todo make file encoding flexible
 */					
					$lines = explode("\n",str_replace("\t", '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;',htmlentities(file_get_contents($e->getFile()),ENT_QUOTES,'UTF-8')));
					
					if (!empty($lines)){
						?>
						<div class="part-title">Code:</div>
						<table class="code">
						<col width="40px">
						<?php
						
							$i = ($e->getLine() > 10)?($e->getLine() - 10):0;
							$n = count($lines);
							$n = ($e->getLine() + 10 < $n)?$e->getLine() + 10:$n;
						 
							while ($i < $n) {
								?>
							<tr<?php echo (($i+1) == $e->getLine()?' class="error-line"':''); ?>><td><?php echo $i+1; ?></td><td class="line"><?php echo $lines[$i];  ?></td></tr>
								<?php
        						$i++;
    						}
						?>
						</table>
						<?php
					}
					?>
					</div>
					<?php
					}
				}break;
			}
			return true;
		}		
	}