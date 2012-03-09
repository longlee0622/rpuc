当前版本的RPUCompiler可以同时输出两种格式的配置信息


对于Cmodel版本
CL0：
	RPU0的4个RCA分别执行一个dfg图称这4个RCA形成一组配置，对应这组配置的CL0文件为 "组号.txt"
CL1:
	RPU0的多组配置只形成一个CL1文件，名为ContextGroupMem_ini.txt
CL2：
	RPU0的多组配置只形成一个CL2文件，名为CoreContextMem_ini.txt
	
对于RTL版本
CL0：
	由于需要加入独立中断控制，每组配置的CL0信息要以CL1Group为单位划分，配置信息统一存放在RPU0_CWI.h
	调用宏分别写在Group组号_link.h文件中
CL1：
	RPU0的多组配置只形成一个CL1文件，名为ContextGroupMem_ini.txt_Data.data
CL2：
	RPU0的多组配置只形成一个CL2文件，名为CoreContextMem_ini.txt_Data.data