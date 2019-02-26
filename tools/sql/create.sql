

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for tbl_account
-- ----------------------------
DROP TABLE IF EXISTS `tbl_account`;
CREATE TABLE `tbl_account` (
  `uid` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `acctname` varchar(255) NOT NULL,
  `pass` varchar(255) NOT NULL,
  PRIMARY KEY (`uid`),
  UNIQUE KEY `acctname` (`acctname`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=100061 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `tbl_account_app`;
CREATE TABLE `tbl_account_app` (
  `uid` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `type` int(11) DEFAULT 0,
  `appid` int(11) NOT NULL ,
  PRIMARY KEY (`uid`),
  UNIQUE KEY `appid` (`appid`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=100000061 DEFAULT CHARSET=utf8;


-- ----------------------------
-- Table structure for tbl_userinfo --`backpack` longblob COMMENT '枪支列表',
-- ----------------------------
DROP TABLE IF EXISTS `tbl_userinfo`;
CREATE TABLE `tbl_userinfo` (
  `uid` int(11) unsigned NOT NULL COMMENT '玩家id',
  `baseinfo` blob ,
  `weapons` blob , 
  `skills` blob,
  `progress` blob,
  `missions` blob,
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*
DROP TABLE IF EXISTS `tbl_weapon`;
CREATE TABLE `tbl_weapon` (
  `uid` int(11) unsigned NOT NULL COMMENT '玩家id',
  `weaponid` int(11) unsigned NOT NULL COMMENT '配置id',
  `bagindex` int(11) unsigned DEFAULT '0' COMMENT '装备位置',
  PRIMARY KEY (`uid`,`weaponid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `tbl_skill`;
CREATE TABLE `tbl_skill` (
  `uid` int(11) unsigned NOT NULL COMMENT '玩家id',
  `skillid` int(11) unsigned NOT NULL,
  `level` int(11) unsigned DEFAULT '0',
  PRIMARY KEY (`uid`,`skillid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for tbl_missionprogress
-- ----------------------------
DROP TABLE IF EXISTS `tbl_missionprogress`;
CREATE TABLE `tbl_missionprogress` (
  `uid` int(11) unsigned NOT NULL COMMENT '玩家id',
  `missiontype` int(11) unsigned NOT NULL COMMENT '任务类型',
  `progress` int(11) unsigned DEFAULT '0',
  `expiretime` int(11) unsigned DEFAULT '0' COMMENT '过期时间',
  PRIMARY KEY (`uid`,`missiontype`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for tbl_mission
-- ----------------------------
DROP TABLE IF EXISTS `tbl_mission`;
CREATE TABLE `tbl_mission` (
  `uid` int(11) unsigned NOT NULL COMMENT '玩家id',     
  `missionid` int(11) unsigned NOT NULL COMMENT '任务id',
  `expiretime` int(11) unsigned DEFAULT '0' COMMENT '到期时间',
   `postion` int(11) unsigned DEFAULT '0' COMMENT '位置',
  PRIMARY KEY (`uid`,`missionid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
*/

