-- Tab最好定成2

do local ret={
['jiermasa']={
  ['cmd']={

  },
  ['name']="西吉尔马萨",
  ['desc']="这里是..."
},
['timbuktu']={
  ['cmd']={

  },
  ['name']="廷巴克图",
  ['desc']="廷巴克图位于北撒哈拉沙漠南缘，西非著名大河尼日尔河中游北岸，按照西非古代游牧民族浪漫的说法，撒哈拉不是沙漠，而是“陆海”，廷巴克图便恰在这“陆海”的中央。对于马里皇帝而言，廷巴克图是他们梦寐以求的宝地，这不仅仅是因为它的商埠地位，更因为它是西非腹地通往麦加的必经之路。盐从北方来，金子从南方来，知识和学问，都得从廷巴克图来”，这句流传至今西非谚语，生动地描述了廷巴克图学术之盛，在全盛时期，廷巴克图号称有20万以上人口，所有街道都从津家里贝尔和桑科尔辐射，构成一个复杂的双中心道路网，在这些蛛网般密布的狭窄街巷深处，隐藏着十几所大学和120座图书馆，大学里不但教授古兰经，还有历史、天文甚至逻辑学。"
},
['ghana']={
  ['cmd']={

  },
  ['name']="加纳",
  ['desc']="根据口头传说，早期的加纳国王是来自北方撒哈拉地区的游牧民族的首领。主要居民是曼丁戈族的索宁克人。阿拉伯地理学家阿尔·法扎里称之为“黄金之国”。国家规定一盎司以上的金块归国王所有，但金砂可以自由买卖。金价有国王控制。据阿尔·巴克希记载：加纳国王对入境的每一头驴所驮的盐，征收1金第纳尔的税，出境税加倍。到加纳访问过的人都盛赞起宫廷中服饰之华丽，不仅国王及大臣的衣服上带有贵重的金饰，就连卫士手中的盾牌、宝剑也镶有黄金。"
},
['sanghai']={
  ['cmd']={

  },
  ['name']="桑海",
  ['desc']="和带有浓厚北非色彩的加纳、马里不同，桑海是纯粹的黑人国家。这个国家的前身是居住着南方尼日尔、贝宁、尼日利亚交界的尼日尔河W转弯处登迪地区的索尔科部落，后来溯尼日尔河而上，在库基亚建立国家，11世纪，都城迁到今天马里境内的加奥，并接受了伊斯兰教，1325年被马里帝国征服（就是“黄金朝圣”的同一年），曼萨.穆萨在朝圣归途经过加奥，将桑海的两位王子阿里·科伦和苏莱曼·纳尔作为人质带往尼阿尼和廷巴克图，两人渡过10年囚徒生涯后先后逃回加奥，并建立了桑海的索尼（Sunni，桑海皇帝称号）王朝，1400年，桑海宣布独立。"
},
['alexander']={
  ['cmd']={
    ['west']=[[
      G.me.at = 'sudan'
      A.look()
    ]]
  },
  ['name']="亚历山大",
  ['desc']="<img src='img/flag_orange.gif' style='float:left' />这里是....."
},
['sudan']={
  ['cmd']={
    ['east']=[[
      G.me.at = 'alexander'
      A.look()
    ]]
  },
  ['name']="sudan",
  ['desc']="这里是..."
}
}return ret end