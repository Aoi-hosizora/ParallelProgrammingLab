# hadoop_wsl_搭建

### 新建新用户

```bash
sudo useradd -m hadoop # 每个节点都必须使用相同的用户名，这里为 hadoop
sudo usermod -aG sudo hadoop # 为 hadoop 用户添加 sudo group
```

### Java

```bash
sudo apt update
sudo apt install openjdk-8-jdk

vim ~/.bashrc
# export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
# export JRE_HOME=${JAVA_HOME}/jre
# export CLASSPATH=.:${JAVA_HOME}/lib:${JRE_HOME}/lib
# export PATH=${JAVA_HOME}/bin:$PATH

source ~/.bashrc
javac -version # 不报错成功
```

### SSH 和 Hosts

```bash
sudo apt install ssh
sudo service ssh start # 如果此处报错 sshd: no hostkeys available -- exiting 则执行下一句后再尝试开启 sshd
# sudo ssh-keygen -A
# 如果在 ~/.ssh 不存在 id_rsa 和 id_rsa.pub 才执行下面这一行，否则直接执行 cat
# ssh-keygen -t rsa # 这里貌似一定要用默认的 id_rsa.pub 文件名，不然 ssh 会报错 localhost: permission denied (publickey)
cat ~/.ssh/id_rsa.pub >> authorized_keys

ssh localhost # 不报错成功，exit 退出即可
```

### Hadoop

```bash
cd /opt
# sudo wget https://dlcdn.apache.org/hadoop/common/hadoop-3.3.2/hadoop-3.3.2-src.tar.gz
sudo wget https://archive.apache.org/dist/hadoop/common/hadoop-2.7.3/hadoop-2.7.3.tar.gz
sudo tar -zxvf hadoop-2.7.3.tar.gz
sudo mv hadoop-2.7.3 hadoop
sudo chown -R hadoop hadoop # 第一个 hadoop 为用户名，第二个 hadoop 为文件夹名

vim ~/.bashrc
# export HADOOP_HOME=/opt/hadoop
# export HADOOP_PREFIXD=/opt/hadoop
# export PATH=$PATH:$HADOOP_HOME/bin:$HADOOP_HOME/sbin

source ~/.bashrc
hadoop version # 不报错
```

### Hadoop Config

```bash
mkdir -p /home/aboutyun/tmp /home/aboutyun/dfs/name /home/aboutyun/dfs/data
cd /opt/hadoop/etc/hadoop
# 按照 https://www.aboutyun.com/forum.php?mod=viewthread&tid=7684 修改以下文件，xml 内容都不用改，JAVA_HOME 为 /usr/lib/jvm/java-8-openjdk-amd64

vim hadoop-env.sh
vim yarn-env.sh
vim core-site.xml # 将 file:/home/aboutyun/tmp 改为 file:/home/hadoop/tmp
vim hdfs-site.xml # 将 file:/home/aboutyun/dfs/xxx 改为 file:/home/hadoop/dfs/xxx
cp mapred-site.xml.template mapred-site.xml
vim mapred-site.xml
vim yarn-site.xml
```

### Master, Slave (先不配置)

```bash
sudo vim /etc/hosts # xxx 和 yyy 看 Windows 下 ipconfig 的 Wireless LAN adapter Wi-Fi 的 IPv4 地址
# 192.168.1.xxx master zdr
# 192.168.1.yyy slave

cd /opt/hadoop/etc/hadoop
vim slaves # 所有 slave 节点
# slave
```

### Run (配置完 Hosts 再运行)

```bash
hdfs namenode -format # 格式化 namenode
start-dfs.sh # 启动 hdfs，stop-dfs.sh 可结束
start-yarn.sh # 启动 yarn，stop-yarn.sh 可结束
jps # 查看当前运行着的进程
# 在 Windows 的浏览器下访问 http://master:8088、http://master:50070、http://slave:8042，可看到 Hadoop 的控制面板
```

### 部分问题记录

+ `hadoop fs` 报错 `Name node is in safe mode`
    + 执行 `hdfs dfsadmin -safemode leave` 退出安全模式，注意必须在 sudoers 下执行，否则会报错 `safemode: Access denied for user hadoop. Superuser privilege is required`
+ ...

### Reference

+ https://www.aboutyun.com/forum.php?mod=viewthread&tid=7684
+ https://www.aboutyun.com//forum.php/?mod=viewthread&tid=7712&extra=page%3D1&page=1&
+ https://www.aboutyun.com//forum.php/?mod=viewthread&tid=7713&extra=page%3D1&page=1&
+ https://www.aboutyun.com//forum.php/?mod=viewthread&tid=7684&extra=page%3D1&page=1&
+ https://www.cnblogs.com/spplus/p/6594396.html
+ https://www.cnblogs.com/zd520pyx1314/p/7246491.html
+ https://blog.csdn.net/whandgdh/article/details/110297373
+ https://blog.csdn.net/sun5966769/article/details/89558088
