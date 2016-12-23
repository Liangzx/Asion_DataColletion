CC = g++
#CC = xlC

ORAINC = -I$(ORACLE_HOME)/rdbms/demo \
	-I$(ORACLE_HOME)/network/public \
	-I$(ORACLE_HOME)/rdbms/public \
	-I$(ORACLE_HOME)/plsql/public \
	-I$(ORACLE_HOME)/precomp/public


PUBLICHOME=/oracle/liangzx/AsionPublicLib

INC = -I.  -I$(PUBLICHOME)/sys \
	-I$(PUBLICHOME)/public \
	-I$(PUBLICHOME)/public_net \
	-I$(PUBLICHOME)/public_oracle \
	-I$(PUBLICHOME)/public_infobus \
	-I$(PUBLICHOME)/public_compress \
	-I /oracle/liangzx/mysrc/publib/hiredis/hiredis-master \
	$(ORAINC) \
	-L/usr/lib64 \

CPPLAG = -D__alpha -D__ORACLECLASS -D__MULTI_THREAD__  -DOTL_ORA_TIMESTAMP -D__ORACLE__ -D__TEST__ -g
#CPPLAG = -D__alpha -D__MULTI_THREAD__ -D__TEST__ -g
LDFLAG = 

DBLIB = -L$(ORACLE_HOME)/lib/ -L$(ORACLE_HOME)/rdbms/lib/
SUNV = -ldl -lnsl -lclntsh


LIBPATH =

.SUFFIXES:.cpp .c

PUBLIC_OBJS = $(PUBLICHOME)/public/pub_string.o \
	$(PUBLICHOME)/public/c_app_error_log.o \
	$(PUBLICHOME)/public/c_app_log.o \
	$(PUBLICHOME)/public/c_application.o \
	$(PUBLICHOME)/public/c_db_index.o \
	$(PUBLICHOME)/public/c_exception.o \
	$(PUBLICHOME)/public/c_file_stream.o \
	$(PUBLICHOME)/public/c_foxdbf.o \
	$(PUBLICHOME)/public/c_indexed_dbf.o \
	$(PUBLICHOME)/public/c_ini_file.o \
	$(PUBLICHOME)/public/c_list.o \
	$(PUBLICHOME)/public/c_list_file.o \
	$(PUBLICHOME)/public/c_string.o \
	$(PUBLICHOME)/public/c_string_list.o \
	$(PUBLICHOME)/public/c_system.o \
	$(PUBLICHOME)/public/c_time.o \
	$(PUBLICHOME)/public/pub_dir_file.o \
	$(PUBLICHOME)/public/app_config.o	

PUBLIC_NET_OBJS = $(PUBLICHOME)/public_net/c_thread_serv_socket.o \
	$(PUBLICHOME)/public_net/c_critical_section.o \
	$(PUBLICHOME)/public_net/c_custom_uni_socket.o \
	$(PUBLICHOME)/public_net/c_server_thread_pool.o \
	$(PUBLICHOME)/public_net/c_simple_event.o \
	$(PUBLICHOME)/public_net/c_thread.o

PUBLIC_ORACLE_OBJS = $(PUBLICHOME)/public_oracle/cmpublic_db.o 

PROJECT_HOME=/oracle/liangzx/DataCollection/src
DATACOLLECTION_OBJS=$(PROJECT_HOME)/DataCollection.o \
	$(PROJECT_HOME)/c_DataCollection_Handle.o \
	$(PROJECT_HOME)/c_DataCollection_PadHandler.o \
	$(PROJECT_HOME)/c_DataCollection_LWHandler.o \
	$(PROJECT_HOME)/c_DataCollection_config.o  \
	$(PROJECT_HOME)/c_DataCollection_JTT808Handler.o \
	$(PROJECT_HOME)/c_DataCollection_server.o \
	$(PROJECT_HOME)/c_DataCollection_log.o \
	$(PROJECT_HOME)/c_DataCollection_task.o


all:DataCollection.bin

.c.o:
	$(CC) $(INC) $(CPPLAG) -c -w $< -o $@ 
	
.cpp.o:
	$(CC) $(INC) $(DBLIB) $(CPPLAG) -c -w $< -o $@

DataCollection.bin:$(PUBLIC_OBJS) $(PUBLIC_NET_OBJS) $(PUBLIC_ORACLE_OBJS) $(DATACOLLECTION_OBJS) 
	$(CC) $(INC) $(DBLIB)  $(CPPLAG) $(LDFLAG) $(SUNV) $(LIBPATH) -o $@  $(PUBLIC_OBJS) $(PUBLIC_ORACLE_OBJS) $(PUBLIC_NET_OBJS)  $(DATACOLLECTION_OBJS) /oracle/liangzx/mysrc/publib/hiredis/hiredis-master/libhiredis.a

clean:
#	rm -f $(PUBLIC_OBJS)
#	rm -f $(PUBLIC_NET_OBJS)
#	rm -f $(PUBLIC_ORACLE_OBJS)
	rm -f $(DATACOLLECTION_OBJS) 
	rm -f DataCollection.bin
	
