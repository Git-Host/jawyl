<?php

interface IKeyValueRepository {
	public function GetValue($nm);
	public function SetValue($nm,$value);
	public function SetValues(array $values);
	public function ListValues();
}