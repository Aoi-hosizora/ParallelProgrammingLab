# hadoop_wsl_搭建

### 新建新用户

```bash
sudo useradd -m hadoop # 每个节点都必须使用相同的用户名，这里为 hadoop
sudo passwd hadoop # 为新用户设置密码
sudo usermod -aG sudo hadoop # 为 hadoop 用户添加 sudo group
# sudo userdel -rf hadoop # 删除用户，以及其相关文件
su hadoop # 进入新用户，输入密码
chsh -s $(which bash) # 更改默认的终端
```

### SSH 免密码登录

```bash
sudo apt install ssh
sudo service ssh start
# 如果上一句报错 sshd: no hostkeys available -- exiting 则执行 "sudo ssh-keygen -A" 后再重试

# 如果先前没有生成过 ~/.ssh/id_rsa 的话才需要执行 ssh-keygen，否则跳过
ssh-keygen -t rsa # 默认情况下，这里要使用默认的 id_rsa.pub 文件名
cat ~/.ssh/id_rsa.pub >> authorized_keys # 将集群中所有节点的公钥都添加到 authorized_keys

ssh localhost # 不报错则成功，exit 退出即可
# 注：可通过在 ~ 下 touch .hushlogin 来隐藏欢迎信息
```

### 配置 Hosts

```bash
sudo cp /etc/hosts /etc/hosts.bak
sudo vim /etc/hosts
# 删除所有项 (不删除 127.* 这些配置的话后面 hadoop jar 运行代码会疯狂 FAILED)，添加以下内容，具体的 IP 和 hostname 自行修改
# 192.168.1.133 master PC-AOIHOSIZORA.localdomain PC-AOIHOSIZORA
# 192.168.1.155 slave1 DESKTOP-RBF32KM.localdomain DESKTOP-RBF32KM
# 192.168.1.149 slave2 SP-AOIHOSIZORA.localdomain SP-AOIHOSIZORA
```

### 安装 JAVA 1.8

```bash
sudo apt update
sudo apt install openjdk-8-jdk
# sudo apt remove --purge openjdk-8-jdk; sudo apt autoremove --purge # 卸载 Java

vim ~/.bashrc
# export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
# export JRE_HOME=${JAVA_HOME}/jre
# export CLASSPATH=.:${JAVA_HOME}/lib:${JRE_HOME}/lib
# export PATH=${JAVA_HOME}/bin:$PATH

source ~/.bashrc
javac -version # 不报错即可
```

### 下载 Hadoop

```bash
cd /opt
sudo wget https://archive.apache.org/dist/hadoop/common/hadoop-2.7.3/hadoop-2.7.3.tar.gz
sudo tar -zxvf hadoop-2.7.3.tar.gz
sudo mv hadoop-2.7.3 hadoop
sudo chown -R hadoop:hadoop hadoop # 第一个 hadoop 为用户名，第二个 hadoop 为用户组名，第三个 hadoop 为文件夹名

vim ~/.bashrc
# export HADOOP_HOME=/opt/hadoop
# export HADOOP_PREFIXD=/opt/hadoop
# export PATH=$PATH:$HADOOP_HOME/bin:$HADOOP_HOME/sbin

source ~/.bashrc
hadoop version # 不报错即可
```

### 配置 Hadoop

```bash
mkdir -p /home/hadoop/hadoop_data
cd /opt/hadoop/etc/hadoop
# 按照 https://www.aboutyun.com/forum.php?mod=viewthread&tid=7684 修改以下文件

vim hadoop-env.sh # JAVA_HOME 的值修改为 /usr/lib/jvm/java-8-openjdk-amd64
vim yarn-env.sh # 取消 export JAVA_HOME 一行的注释，并修改 JAVA_HOME 的值，同上
vim core-site.xml # 粘贴到 <configuration> 内，其中将 file:/home/aboutyun/tmp 改为 file:/home/hadoop/hadoop_data/tmp，将两个 xxx.aboutyun.xxx 改为 xxx.hadoop.xxx
vim hdfs-site.xml # 粘贴到 <configuration> 内，其中将 file:/home/aboutyun/dfs/xxx 改为 file:/home/hadoop/hadoop_data/dfs/xxx
cp mapred-site.xml.template mapred-site.xml
vim mapred-site.xml # 粘贴到 <configuration> 内
vim yarn-site.xml # 粘贴到 <configuration> 内

vim slaves # 添加所有节点的主机名，或 hosts 指定的名称
# master
# slave1
# slave2
```

+ 一些额外配置

```xml
<!-- hdfs-site.xml -->
	<property> <!-- !!! -->
		<name>dfs.socket.timeout</name>
		<value>600000</value>
	</property>

<!-- mapred-site.xml -->
	<property> <!-- !!! -->
		<name>mapreduce.map.maxattempts</name>
		<value>20</value>
	</property>
	<property> <!-- !!! -->
		<name>mapreduce.reduce.maxattempts</name>
		<value>20</value>
	</property>

<!-- yarn-site.xml -->
	<property> <!-- !!! -->
		<name>yarn.nodemanager.vmem-check-enabled</name>
		<value>false</value>
	</property>
```

### Run

```bash
# 注意: 只需要在 master 操作
hdfs namenode -format # 格式化 namenode
start-dfs.sh # 启动 hdfs，stop-dfs.sh 可结束
start-yarn.sh # 启动 yarn，stop-yarn.sh 可结束
jps # 查看当前运行着的进程
# 在 Windows 的浏览器下访问 http://master:8088、http://master:50070、http://slave1:8042，可看到 Hadoop 的控制面板
```

### Reference

+ https://www.aboutyun.com/forum.php?mod=viewthread&tid=7684
+ https://www.aboutyun.com//forum.php/?mod=viewthread&tid=7712&extra=page%3D1&page=1&
+ https://www.aboutyun.com//forum.php/?mod=viewthread&tid=7713&extra=page%3D1&page=1&
+ https://www.aboutyun.com//forum.php/?mod=viewthread&tid=7684&extra=page%3D1&page=1&
+ https://www.cnblogs.com/spplus/p/6594396.html
+ https://www.cnblogs.com/zd520pyx1314/p/7246491.html
+ https://blog.csdn.net/whandgdh/article/details/110297373
+ https://blog.csdn.net/sun5966769/article/details/89558088
