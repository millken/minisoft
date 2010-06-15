-- Tab最好定成2

do local ret={
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