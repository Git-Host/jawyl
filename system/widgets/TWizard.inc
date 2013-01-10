<?php
/**
 * @package View
 * @subpackage Widgets
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

require_once 'common/interfaces.inc';

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * widget displaying widgets in table layout
 */	
	class TWizard extends TJQueryWidget {
		protected function checkSource($src){
			return ($src instanceof IWizardProvider) && parent::checkSource($src);
		}

		public function http_Back(){
			$this->Source->StepBack();
		}
		
		public function http_Perform(){
			$parameters = array();
			$params = $this->Source->StepParameters();
			$args = func_get_args();
			if (!empty($args)){
				$req = $args[count($args) - 1];
				foreach ($params as $p){
					$nm = $p->Name;
					if (isset($req->$nm))
						$parameters[$nm] = $req->$nm;
				}
			}
			$this->Source->StepNext($parameters);
			if ($this->Source->Finished())
				if ($this->FinalRedirect)
					$this->Application()->Redirect($this->FinalRedirect);
		}
		
/**
 * @see TWidget::DefaultOutput()
 */		
		protected function defaultOutput(IThemeEngine $engine){
?>
<div id="<?php echo $this->Name(); ?>" class="ui-widget-wizard">
<?php 
	if ($this->Source->Finished()) {
		?>
	<div class="wiz-final-message"><?php echo $this->SuccessMessage; ?></div> 
		<?php
		return;
	}
?> 
<form id="<?php echo $this->Name(); ?>_frm" method="post">
<input type="hidden" name="controller" value="<?php echo $this->Name(); ?>" />
<input type="hidden" name="method" value="Perform" />
<table>
<?php
	$parameters = $this->Source->StepParameters();
	foreach ($parameters as $p)
		if ($p instanceof IPropertyMeta){
			?>
	<tr>
		<td><?php 
		echo $p->Caption(); 
		?></td><td><?php 
		TJQueryFormAspect::RenderField($this->Name()."_".$p->Name(), $base,$p->Name(),true,$this); 
		?></td>
	</tr>
			<?php
		} 
?>
</table>
</form>
<div class="btn-holder">
<?php if ($this->Source->BackAllowed()){
?>
<form method="post">
<input type="hidden" name="controller" value="<?php echo $this->Name(); ?>" />
<input type="hidden" name="method" value="Back" />
<button onclick="javascript:submit();"><?php __i("Back"); ?></button>
</form>
<?php	
} ?>
<button onclick="javascript:$(&quot;#<?php echo $this->Name(); ?>_frm&quot;).submit();"><?php $this->Source->IsLastStep()?_i("Finish"):_i("Next"); ?></button>
</div>
</div>
<?php			
		}			
	}
?>
