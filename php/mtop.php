<?
/*
 *  mtop-code.php  (ver 06.06.2005)
 *  http://www.mtop.ru/files/mtop_code.zip
 *
 *  Функция предназначена для вставки одного или нескольких текстовых баннеров
 *  в страницу. Данный способ вызова предпочтительный и, в отличие от JavaScript,
 *  практически не вызывает задержек при загрузке страницы
 *
 *  Инструкция:
 *
 *  1. В коде вашей страницы разместите строку
 *
 *     require_once("mtop-code.php");
 *
 *  2. Разместите код вызова, например:
 *
 *     mtop-code(9, 1, 3, '<br><br>');
 *
 *     Первые три параметра будут получены Вами в интерфейсе баннерой
 *     сети (http://www.mtop.ru/user/net/code.php). Последний параметр
 *     означает разделитель, т.е. HTML-код, который будет вставлен между
 *     баннерами.
 *
 *  
 *  Вы можете изменять данный код, если считаете это нужным.
 *
 */


function mtop_code($site_id, $account_id, $banners_num, $div='<br><br>')
{
  global $mtop_code_srand;
  settype($mtop_code_srand, 'integer');
	if(!$mtop_code_srand)
	{
		list($usec, $sec) = explode(' ', microtime());
		mt_srand((float) $sec + ((float) $usec * 100000));
		$mtop_code_srand = 1;
	}

	$ip = ip2long(getenv("REMOTE_ADDR"));
	$fd = fsockopen('nw.mtop.ru', 80, $errno, $errstr, 2);
	if($fd)
	{
		fputs($fd, "GET /show.cgi?4;".$site_id.";".$account_id.";".$banners_num.";$ip HTTP/1.0\r\nHOST: nw.mtop.ru\r\nUser-Agent: phpcode\r\n\r\n");
		while(!feof($fd))	if(($buf = fgets($fd, 1024)) == "\r\n")	break;
		$buf = '';
		while(!feof($fd))
		{
			$buf .= fgets($fd, 1024);
		}
		echo str_replace("\n", $div, $buf);
		fclose($fd);
	}
}
?>