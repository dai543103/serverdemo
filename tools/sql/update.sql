DROP PROCEDURE IF EXISTS `produreSql`;

DELIMITER $$

DROP PROCEDURE IF EXISTS `sp_load_account` ;
CREATE PROCEDURE `sp_load_account`(IN `in_name` varchar(256),IN `in_pass` varchar(256))
BEGIN
	select * from tbl_account where acctname = in_name and pass = in_pass;
END $$

DROP PROCEDURE IF EXISTS `sp_select_account` ;
CREATE PROCEDURE `sp_select_account`(IN `in_name` varchar(256))
BEGIN
	select * from tbl_account where acctname = in_name;
END $$

DROP PROCEDURE IF EXISTS `sp_insert_account` ;
CREATE PROCEDURE `sp_insert_account`(IN `in_name` varchar(256),IN `in_pass` varchar(256))
BEGIN
	insert into tbl_account(acctname, pass) values (in_name,in_pass);
END $$

DROP PROCEDURE IF EXISTS `sp_select_account_app` ;
CREATE PROCEDURE `sp_select_account_app`(IN `in_id` int)
BEGIN
	select * from tbl_account_app where appid = in_id ;
END $$


DROP PROCEDURE IF EXISTS `sp_insert_account_app` ;
CREATE PROCEDURE `sp_insert_account_app`(IN `in_id` int)
BEGIN
	insert into tbl_account_app(appid) values (in_id);
END $$

DELIMITER ;
