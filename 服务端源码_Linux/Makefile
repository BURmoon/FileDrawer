
# 使用的编译器
CC=gcc
# 预处理参数
CPPLFAGS=-I./include					\
		 -I/usr/include/fastdfs			\
		 -I/usr/include/fastcommon		\
		 -I/usr/local/include/hiredis/  \
		 -I/usr/local/mysql/include		
# 选项
CFLAGS=-Wall
# 需要链接的动态库
LIBS=-lmysqlclient  \
	 -lfdfsclient	\
	 -lfastcommon	\
	 -lhiredis		\
	 -lfcgi         \
	 -lm            
# 目录路径
COMMON_PATH=common
CGI_BIN_PATH=bin_cgi
CGI_SRC_PATH=src_cgi

# 子目标
login=$(CGI_BIN_PATH)/login
register=$(CGI_BIN_PATH)/register
upload=$(CGI_BIN_PATH)/upload
md5=$(CGI_BIN_PATH)/md5
myfiles=$(CGI_BIN_PATH)/myfiles
dealfile=$(CGI_BIN_PATH)/dealfile

# 最终目标
target=$(login)			\
	   $(register)		\
	   $(upload)		\
	   $(md5)			\
	   $(myfiles)		\
	   $(dealfile)		\
	   $(sharefiles)	\
	   $(dealsharefile)
ALL:$(target)

# 登录
$(login):	$(CGI_SRC_PATH)/login_cgi.o \
			$(COMMON_PATH)/cJSON.o \
			$(COMMON_PATH)/deal_mysql.o \
			$(COMMON_PATH)/redis_op.o  \
			$(COMMON_PATH)/cfg.o \
			$(COMMON_PATH)/util_cgi.o \
			$(COMMON_PATH)/des.o \
			$(COMMON_PATH)/base64.o \
			$(COMMON_PATH)/md5.o  \
			$(COMMON_PATH)/make_log.o  
	$(CC) $^ -o $@ -L /usr/bin $(LIBS)

# 注册
$(register):	$(CGI_SRC_PATH)/reg_cgi.o \
				$(COMMON_PATH)/util_cgi.o \
				$(COMMON_PATH)/cJSON.o \
				$(COMMON_PATH)/deal_mysql.o \
				$(COMMON_PATH)/redis_op.o  \
				$(COMMON_PATH)/cfg.o   \
				$(COMMON_PATH)/make_log.o  
	$(CC) $^ -o $@ -L /usr/bin $(LIBS)

# 秒传
$(md5):		$(CGI_SRC_PATH)/md5_cgi.o \
			$(COMMON_PATH)/util_cgi.o \
			$(COMMON_PATH)/cJSON.o \
			$(COMMON_PATH)/deal_mysql.o \
			$(COMMON_PATH)/redis_op.o  \
			$(COMMON_PATH)/cfg.o   \
			$(COMMON_PATH)/make_log.o
	$(CC) $^ -o $@ -L /usr/bin $(LIBS)

# 上传
$(upload):$(CGI_SRC_PATH)/upload_cgi.o \
		  $(COMMON_PATH)/util_cgi.o \
		  $(COMMON_PATH)/cJSON.o \
		  $(COMMON_PATH)/deal_mysql.o \
		  $(COMMON_PATH)/redis_op.o  \
		  $(COMMON_PATH)/cfg.o\
		  $(COMMON_PATH)/make_log.o
	$(CC) $^ -o $@ -L /usr/bin $(LIBS)

# 用户文件列表
$(myfiles):	$(CGI_SRC_PATH)/myfiles_cgi.o \
			$(COMMON_PATH)/util_cgi.o \
			$(COMMON_PATH)/cJSON.o \
			$(COMMON_PATH)/deal_mysql.o \
			$(COMMON_PATH)/redis_op.o  \
			$(COMMON_PATH)/cfg.o\
			$(COMMON_PATH)/make_log.o
	$(CC) $^ -o $@ -L /usr/bin $(LIBS)

# 删除文件、pv字段处理
$(dealfile):$(CGI_SRC_PATH)/dealfile_cgi.o \
			$(COMMON_PATH)/util_cgi.o \
			$(COMMON_PATH)/cJSON.o \
			$(COMMON_PATH)/deal_mysql.o \
			$(COMMON_PATH)/redis_op.o  \
			$(COMMON_PATH)/cfg.o\
			$(COMMON_PATH)/make_log.o
	$(CC) $^ -o $@ -L /usr/bin $(LIBS)

# 生成所有的.o 文件
%.o:%.c
	$(CC) -c $< -o $@ $(CPPLFAGS) $(CFLAGS)

clean:
	-rm -rf *.o $(target) $(CGI_SRC_PATH)/*.o $(COMMON_PATH)/*.o
.PHONY:clean ALL

