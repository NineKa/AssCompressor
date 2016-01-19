# ==== AID2CID ==== #
aid是显示在地址栏中的数字
例如：http://www.bilibili.com/video/av1696203/
aid=1696203

cid是某段视频的唯一编号
同一个aid会因为分P对应多个cid

def aid2cid(aid,page):
  appkey=[DATA DELETED]
  appsecret=[DATA DELETED]
  args='appkey={0}&id={1}&page={2}&type=json'.format(appkey,aid,page)
  sign=hashlib.md5((args+appsecret).encode()).hexdigest()
  url='http://api.bilibili.tv/view?{0}&sign={1}'.format(args,sign)
  return json.loads(HttpRequest(url))['cid']

当然也可以直接抓网页正则。。。。。

# ==== 历史弹幕 ==== #

cid=430882

http://comment.bilibili.com/rolldate,430882
http://comment.bilibili.com/dmroll,1373731200,430882
http://comment.bilibili.com/430882.xml

# ==== BiliBili XML ==== #
<?xml version="1.0" encoding="UTF-8"?>
<i>
  <chatserver>chat.bilibili.com</chatserver>
  <chatid>430882</chatid>
  <mission>0</mission>
  <maxlimit>1500</maxlimit>
  <source>k-v</source>
  <d p="2.2999999523163,1,25,16777215,1337574323,0,1f32ad99,93168927">[Context here]</d>
  <d p="17.600000038147,1,25,16777215,1337965177,0,fe047a09,94120346">[Another Comment]</d>
</i>

  #p属性解释：逗号分割，共8项
  1. 该条弹幕出现在视频上的时间（相对于视频开始），单位秒
  2. 弹幕类型，整数
  3. 字体大小，整数，默认大小为25（https://github.com/jabbany/CommentCoreLibrary/blob/de9f6a203fb58e3c763061a09e7789e94c389b95/docs/CommentSizes.md）
  4. 字体颜色，以B站方式存储，整数
  5. 该条弹幕发送时的UNIX时间戳，整数
  6. 所在弹幕池，整数，0=普通弹幕，1=？
  7. 用户ID，发送该弹幕的用户ID，疑似16进制字符串
  8. 弹幕ID，该弹幕的唯一ID，疑似整数

  # 字体颜色存储的B站方式：
  ASS以BGR表示颜色
  B站以RGB顺序存储
  unsigned char red,green,blue;
  color_b=(red<<32)&(green<<16)&(blue)

  # 弹幕类型对应：
  * 也见https://github.com/jabbany/CommentCoreLibrary/blob/de9f6a203fb58e3c763061a09e7789e94c389b95/docs/CommentTypes.md
  - 滚动弹幕(右向左) == 1
  - 下端滚动弹幕 == 2
  - 底部固定弹幕 == 4
  - 顶部固定弹幕 == 5
  - 逆向滚动弹幕 == 6
  - 特殊弹幕 == 7 （固定位置/移动/轨迹）
  - 高级弹幕 == 8 （代码弹幕）

  # B站座标系约定
  屏幕向右为X轴正方向
  屏幕向下为Y轴正方向
  屏幕向内为Z轴正方向
  原点在屏幕左上角

# ==== 特殊弹幕内容 ==== #

特殊弹幕的d标签为一json数组
因B站多次改版，存在多种格式:
       |0  |1  |2             |3                |4      |5       |6       |7   |8   |9                  |10              |11       |12             |13             |14        |15
ACC-05 [X  ,Y  ,fadein-fadeout,duration         ,content]        |        |    |    |                   |                |   here  |               |               |          |
ACC-07 [X  ,Y  ,fadein-fadeout,duration         ,content,z-rotate,y-rotate]    |    |                   |                |different|               |               |          |
ACC-11 [X  ,Y  ,fadein-fadeout,duration         ,content,z-rotate,y-rotate,to-x,to-y,moving-duration    ,moving-pause]   |         |               |               |          |
ACC-13 [X  ,Y  ,fadein-fadeout,duration         ,content,z-rotate,y-rotate,to-x,to-y,moving-duration    ,moving-pause    ,font-name,linear-movement]               |          |
ACC-14 [X  ,Y  ,fadein-fadeout,duration         ,content,z-rotate,y-rotate,to-x,to-y,moving-duration    ,moving-pause    ,highlight,font-name      ,linear-movement]          |
ACC-15 [X  ,Y  ,fadein-fadeout,duration(seconds),content,z-rotate,y-rotate,to-x,to-y,moving-duration(ms),moving-pause(ms),highlight,font-name      ,linear-movement,trace-line]


- X: 弹幕左上角X座标
- Y: 弹幕左上角Y座标
- fadein-fadeout: 透明度，两个0~1之间的浮点数，以`-`连接。
                  1=不透明 ； 0=全透明，
                  fade-in: 弹幕出现时的透明度
                  fade-out:弹幕消失时的透明度
                  随时间线性变化
- duration 从出现到消失的时间
- content 内容
- z-rotate 角度表示，Z轴旋转（详见“弹幕透视”）
- y-rotate 角度表示，Y轴旋转（详见“弹幕透视”）
- to-x 移动目标点 X
- to-y 移动目标点 Y
- moving-duration 移动所花时间，单位millisecond [10E-3 s]
- moving-pause 开始移动前的暂停时间，单位millisecond
- highlight 描边，true为描边，false为不描边。除非为纯黑色(#000000)描白边，否则描黑边
- font-name 字体名称（？）
- linear-movement 线性移动 1=匀速运动 0=(匀？)减速运动
- trace-line 轨迹弹幕标记点列表 `M515,307L516,305L518,303L519,302`
             M{x0},{y0}L{x1},{y1}L{x2},{y2}.........

  # 轨迹弹幕标记点列表格式

# ==== 弹幕透视 ==== #
Z轴旋转正角度为：右手握Z轴，大拇指指向Z轴正方向，按四指方向绕弹幕左上角旋转
Y轴旋转正角度为：右手握Y轴，大拇指指向Y轴正方向，按四指方向绕弹幕左上角旋转 (小于180度时在屏幕外，大于180度时在屏幕内)
同时旋转Z和Y轴时，先完成Y轴旋转，再完成Z轴旋转。


# ==== ASS相关实现方法和注意点 ==== #


# === 游客弹幕 === #
游客弹幕的userID 已 D 开头


Heavy Barrier Test:
av314
av5878
av374672  Mode7 Also
av332732