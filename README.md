# 2017-Network-Programming-Practic6-24Apr2017

2017/4/24 In-class	practice #6 <br>
Time:	~	21:30 <br>
Please	use select function	to	implement	TCP	and	UDP	co-existence Echo	Server. <br>
Check	1(50%):	TCP	and	UCP	client	can	connect	server	and	reply	message	simultaneously.  <br>
Check	2(10%):	TCP	can	handle	multiple	clients (using	fork	function)  <br>
Check	3(20%):	Signal	handling	function (avoid	zombie	and	interrupt)  <br>
Check	4(20%):	Show	client/server	connection	information  <br> <br>
Ex:	  <br>
Client	$tcpclient	[server	IP]	hello <br>
hello	from	[server	IP] <br> <br>
Demo	steps: <br>
Server	$tuserver	[server	IP] <br>
connection	from	[client	IP] port child	pid by	TCP <br>
receives	hello <br>
connection	from	[client	IP] port by	UDP <br> <br>
Bonus(20%,	only	today):	build	a	TCP	and	UDP	co-existence Echo	Client <br>
Ex:	 <br>
Client	$tuclient TCP	[server	IP]	hello <br>
hello <br>
Client	$tuclient	UDP	[server	IP]	hello <br>
hello <br>
