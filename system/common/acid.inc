<?php 
interface ITransactionProvider {
/**
 * starts transaction
 * @return bool
 */			
	public function BeginTransaction();
/**
 * commits transaction
 * @return bool
 */		
	public function CommitTransaction();
/**
 * rollbacks transaction
 * @return bool
 */		
	public function RollbackTransaction();	
}