
SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for file_info
-- ----------------------------
DROP TABLE IF EXISTS `file_info`;
CREATE TABLE `file_info` (
  `md5` varchar(200) NOT NULL,
  `file_id` varchar(256) NOT NULL,
  `url` varchar(512) NOT NULL,
  `size` bigint(20) DEFAULT NULL,
  `type` varchar(20) DEFAULT NULL,
  `count` int(11) DEFAULT NULL,
  PRIMARY KEY (`md5`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of file_info
-- ----------------------------

-- ----------------------------
-- Table structure for user
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user` (
  `name` varchar(128) NOT NULL,
  `password` varchar(128) NOT NULL,
  PRIMARY KEY (`name`),
  UNIQUE KEY `uq_name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=49 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of user
-- ----------------------------

-- ----------------------------
-- Table structure for user_file_count
-- ----------------------------
DROP TABLE IF EXISTS `user_file_count`;
CREATE TABLE `user_file_count` (
  `user` varchar(128) NOT NULL,
  `count` int(11) DEFAULT NULL,
  PRIMARY KEY (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of user_file_count
-- ----------------------------

-- ----------------------------
-- Table structure for user_file_list
-- ----------------------------
DROP TABLE IF EXISTS `user_file_list`;
CREATE TABLE `user_file_list` (
  `user` varchar(128) NOT NULL,
  `md5` varchar(200) NOT NULL,
  `createtime` varchar(128) DEFAULT NULL,
  `filename` varchar(128) DEFAULT NULL,
  `pv` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of user_file_list
-- ----------------------------
SET FOREIGN_KEY_CHECKS=1;
