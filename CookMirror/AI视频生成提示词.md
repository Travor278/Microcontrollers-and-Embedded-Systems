# CookMirror 厨魔镜 — AI 视频生成提示词集

> 适用模型:**Sora 2 / 可灵 2.0 / Veo 3 / Pika 2.0 / Runway Gen-4**
> 每个场景提供中英文版本,英文版本适配 Sora/Veo,中文版本适配可灵/Pika 国内版。
> 时长建议:每条 8–12 秒,可拼接为 90s 产品短片。

---

## 📋 视频整体结构(90s 产品片)

| 序号 | 场景 | 时长 | 情绪 | 转场到下一镜 |
| --- | --- | --- | --- | --- |
| 1 | 产品形象 Hero Shot | 8s | 科技感 / 优雅 | 镜面蓝色光晕扩散 → 新厨房 |
| 2 | AR 投影指导烹饪 | 10s | 惊叹 / 实用 | 黄色 AR 箭头变红 → 热感扩散 |
| 3 | 红外锅温实时显示 | 8s | 专业 / 安心 | 数字跳红 → 警报闪光淹没画面 |
| 4 | 火灾自动关火 | 10s | 紧张转安心 | 绿色"已关火"光圈 → 平静日常 |
| 5 | 隔空手势翻页 | 8s | 酷炫 / 巧思 | 滑动手势延续 → 菜谱滑出 |
| 6 | 婴幼儿辅食 / 孕妇营养 | 10s | 温馨 / 关怀 | 镜面倒影 morph 时空切换 |
| 7 | 家庭食谱传承 | 12s | 感动 / 催泪 | 泪光接镜面蓝光 → 分屏 |
| 8 | 异地同步做饭 | 10s | 思念 / 团圆 | 双窗夕阳合并 → 拉远剖面 |
| 9 | 多产品生态全景 | 8s | 宏大 / 未来 | 三房光晕汇聚到中央 |
| 10 | LOGO 定版 | 6s | 品牌沉淀 | — |

---

## 🌀 统一转场主题:「镜面光晕」(Glass Bloom)

> 全片转场用同一个视觉符号——**镜面中心蓝色光晕向外扩散覆盖画面 → 在下一场景的镜面中心重新聚拢**。
> 既符合产品特性(镜面=主角),又像 Apple 产品片那样有"统一气韵"。配同一个"嗡——"的科技音贯穿全片。

**通用转场提示词模板**(粘到任意场景末尾):

中文版:

镜头末尾,镜面中央蓝色光晕缓缓向外扩散,直至覆盖整个画面,画面变为纯净蓝光过渡帧,持续 0.5 秒后光晕从中心快速收缩,重新聚焦到下一场景的镜面中央,自然衔接。

English:

At the shot's end, the blue glow from the mirror's center expands outward until it engulfs the entire frame. Hold a pure blue transition frame for 0.5 seconds, then the glow rapidly contracts to converge into the center of the next scene's mirror, seamlessly bridging the cut.

---

## 🎬 场景 1:产品形象 Hero Shot

### 中文(可灵/Pika)

现代化简约厨房,清晨柔和阳光从右侧窗户洒入。镜头从厨房入口缓缓推进,聚焦油烟机正下方挂着一面 60×80cm 的黑色边框智能镜面。镜面初始为纯镜面状态,清晰反射厨房景象。镜头推近至特写,镜面内部蓝色光晕缓缓点亮,浮现出极简风格 UI:"早安,今天想做什么?",字体优雅。镜头略微上扬展示整体形态,镜面右侧缓缓显示当日推荐菜谱卡片。画面色调温暖中带科技蓝光晕,景深柔和,4K 电影级质感,慢镜头推进 8 秒。

### English (Sora/Veo)

Modern minimalist kitchen at sunrise. Warm soft light streams through a right-side window. Camera slowly dollies in from the kitchen doorway, focusing on a 60x80cm black-bezel smart mirror mounted under the range hood. The mirror is initially in pure reflective state, perfectly reflecting the kitchen. Camera pushes to close-up. A subtle blue glow softly illuminates from within the mirror, revealing an elegant minimalist UI: "Good morning. What shall we cook today?" The camera tilts slightly up to reveal the full product form. A recipe card slides in from the right side of the mirror. Cinematic color grade: warm tones with subtle tech blue accents, shallow depth of field, 4K filmic quality, slow dolly-in, 8 seconds.

### 🔄 转场 1→2(末尾追加)

- **中文**:镜头末尾,镜面 UI 蓝色光晕从中心缓缓向外扩散,直至完全覆盖画面;0.5 秒纯蓝过渡帧后,光晕在新场景的另一面厨房镜面中央重新聚拢,显露出 AR 指导的菜谱界面。
- **EN**:At the end, the mirror's blue UI glow expands outward from center until it fully engulfs the frame. After a 0.5s pure-blue transition frame, the glow converges into the center of a different kitchen mirror in the next scene, revealing the AR cooking guidance interface.

---

## 🎬 场景 2:AR 投影指导烹饪

### 中文

一位 30 岁年轻女性在灶台前准备炒菜。镜头俯拍视角:灶台上有一口黑色平底锅,旁边摆放葱姜蒜末小碟和切好的肉片。CookMirror 镜面位于画面上方,镜面浮现出半透明虚拟黄色箭头,精准指向小碟中的葱花,旁边浮现文字"先下葱花爆香"。女性按指引将葱花倒入热锅,镜面立即更新,箭头转向肉片碟,文字变为"30 秒后加入肉片"。镜面右上角实时显示"油温 175℃ ✓"。整个画面流畅自然,箭头投影具有半透明全息感,与现实空间精准对齐。4K 电影级质感,镜头微微推进,10 秒。

### English

A young woman in her 30s stands at the stove preparing to cook. Overhead shot: a black skillet on the stovetop, small dishes of chopped scallions, ginger, garlic, and sliced meat beside it. The CookMirror sits in the upper frame, displaying a semi-transparent yellow holographic arrow pointing precisely at the scallion dish, with text "Add scallions first." The woman follows the cue and pours scallions into the hot pan. The mirror instantly updates: the arrow swings to the meat dish, text changes to "Add meat in 30 seconds." Upper right of mirror shows "Oil temp 175C ✓" in real-time. Holographic AR arrows blend seamlessly with reality, with accurate spatial registration. 4K cinematic quality, slow push-in, 10 seconds.

### 🔄 转场 2→3

- **中文**:镜头末尾,镜面上的黄色 AR 箭头颜色逐渐变橙再变红;一圈红色热感涟漪从镜面中心向外扩散,瞬间将画面染成温热橙红色,自然过渡到红外热成像视图。
- **EN**:At the end, the yellow AR arrow on the mirror gradually shifts to orange then deep red. A red heat-ripple expands outward from the mirror's center, instantly washing the frame in warm orange-red tones, naturally bridging into the infrared thermal view.

---

## 🎬 场景 3:红外锅温实时显示(技术力)

### 中文

锅中油已加热,镜头特写锅底翻滚的油花。画面分屏切换:左侧实拍油锅,右侧 CookMirror 镜面显示红外热成像图,锅心呈现红黄色高温区,温度数字"178℃"实时跳动。当油温升至 200℃ 时,镜面数字变红,弹出黄色警告框"⚠️ 油温过高,即将冒烟,建议下锅",同时画外传出柔和女声提示音。画面有科技感和专业感,色彩对比强烈,8 秒。

### English

Cooking oil heats up in a pan. Close-up of swirling oil bubbles. Split screen: left shows the actual pan, right shows CookMirror displaying an infrared thermal map with the pan center glowing red-yellow, temperature "178C" updating in real-time. As oil temp rises to 200C, the number turns red, a yellow warning popup appears "Oil temp critical, smoke imminent. Suggest adding ingredients now," accompanied by a soft female voice alert. High-tech professional aesthetic, strong color contrast, 8 seconds.

### 🔄 转场 3→4

- **中文**:镜面温度数字从 178℃ 飞速跳到 200℃,数字闪红;红色警报闪光从屏幕中心爆开,瞬间淹没整个画面;0.3 秒纯红警报帧后,画面切换到妈妈在客厅接电话的静谧场景,形成强烈反差。
- **EN**:Temperature on the mirror leaps from 178C to 200C, digits flashing red. A crimson alarm flash explodes from the screen center, instantly flooding the entire frame. After a 0.3s pure-red alert frame, the scene cuts sharply to the mother answering a phone call in the quiet living room — a stark contrast.

---

## 🎬 场景 4:火灾自动关火(安全卖点 - 戏剧化)

### 中文

一位中年妈妈接电话走出厨房,留下锅子在灶上空烧。镜头静止拍摄灶台,3 秒空镜后,锅子开始冒出青烟,锅底发红。突然 CookMirror 镜面变为红色警报界面,显示"⚠️ 空锅干烧 检测到 警告!",90 分贝蜂鸣声响起,同时镜头快速摇向燃气阀,特写电磁阀"咔"地一声自动关闭,火焰瞬间熄灭。妈妈听到警报匆忙跑回厨房,看到镜面提示"✓ 已自动关火,请检查锅具",表情从惊慌转为安心。整体画面紧张感强,色调从灰冷转暖,镜头切换快速,10 秒。

### English

A middle-aged mother walks out of the kitchen to answer a phone call, leaving a pan on the active burner. Static shot of the stovetop. After 3 seconds of silence, smoke rises from the pan, the bottom glows red. Suddenly the CookMirror flashes red alarm interface: "DRY-BURN DETECTED  WARNING!" A 90dB beeper sounds. Quick whip-pan to the gas valve: close-up of the electromagnetic valve clicking shut, flames extinguishing instantly. The mother rushes back into the kitchen, sees the mirror prompt "Auto shut-off complete. Please check cookware," her expression shifts from panic to relief. Tense atmosphere, cold-to-warm color shift, fast cuts, 10 seconds.

### 🔄 转场 4→5

- **中文**:镜面显示的"✓ 已自动关火"绿色对勾缓缓放大,化作一圈柔和的绿色光圈向外扩散覆盖画面;光圈消散后,画面切换到几天后阳光明媚的厨房,一位男性厨师正在轻松揉面,情绪从紧张转为日常温馨。
- **EN**:The green checkmark "Auto shut-off complete" on the mirror slowly enlarges, transforming into a soft green ring that expands outward to cover the frame. As the ring dissipates, the scene cuts to a sunny kitchen days later, where a male chef calmly kneads dough — emotion shifts from tension to peaceful everyday warmth.

---

## 🎬 场景 5:隔空手势翻页(交互创新)

### 中文

一位男性厨师双手沾满面粉揉面,镜头特写他白花花的手掌。他抬头看向CookMirror,镜面显示当前菜谱步骤 3/8。男子将沾满面粉的手悬空,对着镜面做出从右向左的滑动手势,镜面立即翻页至步骤 4/8,画面切换流畅自然,菜谱卡片有滑入动画。男子满意微笑继续揉面,完全无需触碰任何设备。镜头采用 35mm 中焦,虚化背景,强调手势与镜面的隔空交互美感,8 秒。

### English

A male chef kneads dough, hands covered in flour. Close-up of his white floury palms. He looks up at the CookMirror displaying recipe step 3/8. With dough-covered hands, he performs a right-to-left swipe gesture in mid-air toward the mirror. The mirror instantly pages to step 4/8 with a smooth slide animation. He smiles in satisfaction and continues kneading—no device contact needed. 35mm medium focus, blurred background, emphasizing the elegance of mid-air gesture-to-mirror interaction, 8 seconds.
```

### 🔄 转场 5→6

- **中文**:男厨师的滑动手势继续延伸到下一帧,菜谱卡片从右向左持续滑出画面;滑动过程中卡片内容从"揉面步骤"变换为"婴幼儿辅食 · 8 月龄";镜头随之拉远,显露出一位年轻妈妈抱着宝宝站在新场景的厨房中。
- **EN**:The chef's swipe gesture continues across the cut into the next frame, the recipe card sliding off-screen from right to left. Mid-slide, the card content morphs from "kneading step" into "Baby Food  8-month." The camera pulls back to reveal a young mother holding her baby in the new kitchen scene.

---

## 🎬 场景 6:婴幼儿辅食 / 孕妇营养(健康关怀)

### 中文

一位年轻妈妈抱着 8 个月大的宝宝站在厨房中,宝宝穿着浅黄色围嘴。CookMirror 镜面显示婴幼儿辅食专属界面,卡片上写"8 月龄 · 南瓜胡萝卜泥"。镜面右侧弹出营养面板:"含 β-胡萝卜素 ↑,无过敏原 ✓,质地:细腻泥状"。镜头切换至灶台,妈妈将蒸熟的南瓜胡萝卜放入辅食机,镜面实时显示进度条。最后镜头切到宝宝坐在餐椅上张嘴吃辅食,妈妈用手机扫描盘中食物,镜面跳出"本餐 ✓ 已记录,本周 Fe 摄入 +5%"。画面温馨柔和,以暖黄色调为主,10 秒。


### English

A young mother holds her 8-month-old baby in the kitchen, baby wearing a pale yellow bib. The CookMirror displays a baby food interface: "8-month  Pumpkin Carrot Puree." A nutrition panel slides in: "Beta-carotene high. No allergens. Texture: smooth puree." Cut to the stovetop: mother places steamed pumpkin and carrot into a baby food processor, mirror shows real-time progress bar. Final cut: baby sitting in a high chair eagerly opening mouth for the puree, mother scans the plate with her phone, mirror pops up "Meal logged. Weekly Fe intake +5%." Warm soft yellow color grade, intimate atmosphere, 10 seconds.

### 🔄 转场 6→7(全片情感高潮的关键转场)

- **中文**:镜头特写镜面中 30 岁妈妈温柔的倒影;倒影逐渐发生 morph 变化——皱纹缓缓浮现、黑发渐变为银发、面容慢慢叠化为 70 岁奶奶的模样;时空在镜面中无声切换,背景音乐响起钢琴主题旋律,过渡到奶奶炒红烧肉的场景。
- **EN**:Close-up on the mirror's reflection of the 30-year-old mother's gentle face. The reflection slowly morphs — wrinkles emerge, dark hair fades to silver, features dissolve into the 70-year-old grandmother's visage. Time and space transition silently within the mirror as a piano theme swells, leading into the grandmother cooking braised pork.

---

## 🎬 场景 7:家庭食谱传承(催泪情感)

### 中文

昏黄温暖的厨房灯光下,一位 70 岁满头银发的奶奶系着围裙,正在炒一道红烧肉。CookMirror 镜面以"传承模式"运行,屏幕分上下两栏:上栏实时绿色框线追踪奶奶的手部动作,显示"翻面 ✓ 加冰糖 ✓小火慢炖 30 分钟 ✓";下栏自动生成数字食谱,文字一行行浮现"奶奶的红烧肉 · 第 1 代秘方"。镜头切换:几年后,同一个厨房,长大的孙女(20 岁)独自站在锅前,镜面播放奶奶当年的做菜视频,旁边显示步骤同步。孙女眼眶湿润,嘴角带笑,锅中红烧肉冒着热气。最后镜头特写镜面,显示"奶奶的红烧肉 · 第 2 代传承"。画面催泪感强,色调温暖怀旧,12 秒。
```

### English

A 70-year-old silver-haired grandmother in a warm dimly-lit kitchen, wearing an apron, cooks braised pork. The CookMirror runs in "Heritage Mode," screen split into two: top half tracks her hand movements with green outlines, displaying "Flip done. Rock sugar added. Slow simmer 30min done"; bottom half generates a digital recipe with text appearing line by line: "Grandma's Braised Pork  Generation 1." Cut to years later, same kitchen: the granddaughter, now 20, stands alone at the stove. The mirror plays grandma's original cooking video alongside synchronized steps. Granddaughter's eyes well up, a smile crosses her lips, steam rises from the pot. Final close-up of mirror: "Grandma's Braised Pork Generation 2 Inherited." Deeply emotional, warm nostalgic color grade, 12 seconds.
```

### 🔄 转场 7→8

- **中文**:孙女眼眶湿润的特写,泪光中倒映出镜面的蓝色光晕;光晕越来越亮,从眼眸中迸发,瞬间扩展成一条对角线将画面一分为二;两半画面分别呈现两个不同城市的厨房,过渡到母女异地同步做饭的分屏画面。
- **EN**:Close-up on the granddaughter's tear-filled eyes, with the mirror's blue glow reflected in her tears. The glow grows brighter, bursts from her eyes, and instantly expands into a diagonal line splitting the frame in two. The halves reveal two kitchens in different cities, transitioning into the split-screen of mother and daughter cooking together remotely.

---

## 🎬 场景 8:异地同步做饭(亲情远程)

### 中文

分屏画面:左侧是上海公寓的厨房,女儿(25 岁)站在 CookMirror 前;右侧是老家小镇厨房,妈妈(55 岁)站在另一面 CookMirror 前。两面镜面同步显示同一道菜"番茄牛腩"步骤 5/10。妈妈对着镜面说话"乖,牛腩要先焯水再炖,记得吗?",声音通过女儿一侧的镜面喇叭清晰传出。女儿笑着点头"妈我学会啦",镜头中女儿翻动锅铲,妈妈一侧镜面同步出现女儿的实时画面。两位都在做饭,但通过镜面仿佛在同一个厨房。窗外都是傍晚的橘红夕阳。10 秒,温暖团圆感。


### English

Split screen: left side shows a Shanghai apartment kitchen, daughter (25) standing before her CookMirror; right side shows a small-town home kitchen, mother (55) standing before another CookMirror. Both mirrors synchronously display the same recipe "Tomato Beef Brisket  Step 5/10." Mother speaks toward her mirror: "Sweetie, blanch the brisket first before stewing, remember?" Her voice clearly emerges from the daughter's mirror speaker. The daughter smiles and nods, "Got it, mom." She stirs the pot; mother's mirror simultaneously shows daughter's live video. Both cooking, yet connected as if in the same kitchen. Both windows reveal orange sunset light. 10 seconds, warm reunion vibe.
```

### 🔄 转场 8→9

- **中文**:分屏两侧窗外的橘红色夕阳缓缓向画面中央移动,最终在中线合二为一,形成一轮完整的落日;镜头从这轮夕阳快速向后拉远,穿越窗户飞出室外,显露出整栋房屋的剖面全景,过渡到三个房间的生态视图。
- **EN**:The orange sunsets outside both split-screen windows slowly drift toward the center, finally merging into one complete setting sun. The camera then rapidly pulls back from this sun, flying out through the window to reveal the entire house's cross-section, transitioning into the three-room ecosystem view.

---

## 🎬 场景 9:多产品生态全景(立意拔高)

### 中文

一个未来感家庭场景的剖面图,镜头缓缓平移展示三个房间:
- 客厅:健身镜前一位男性正在锻炼,镜面显示骨骼追踪线条
- 厨房:CookMirror 前妻子做晚餐,镜面 AR 箭头指引
- 婴儿房:婴儿床上方雷达守护仪发出柔和绿光,母亲手机推送"宝宝平稳入睡"三个房间通过半透明数据流光线连接,汇入中央云端图标"AI 家庭健康生态"。整体画面有未来感和宏大叙事感,色调冷蓝带暖光点缀。8 秒慢镜头横移。

### English

Cutaway cross-section view of a futuristic family home. Camera slowly
pans across three rooms:
- Living room: a man exercises before a FitMirror displaying skeletal
  tracking lines
- Kitchen: wife prepares dinner before CookMirror with AR arrow guidance
- Nursery: a guardian radar above the crib glows soft green, mother's phone shows "Baby sleeping peacefully"
Three rooms connected by translucent flowing data streams converging into a central cloud icon labeled "AI Family Wellness Ecosystem." Grand futuristic narrative, cool blue with warm accent lighting, 8-second slow lateral pan.
```

### 🔄 转场 9→10

- **中文**:三个房间的数据流光线持续向中央汇聚,最终在画面中心凝聚成一团明亮的蓝色光球;光球旋转中放出耀眼光芒,瞬间吞没全部画面;光芒褪去后,纯黑背景中央 CookMirror 产品轮廓缓缓显现,进入 LOGO 定版。
- **EN**:Streams of data from all three rooms continue converging toward the center, finally coalescing into a brilliant blue orb at the frame's heart. The orb spins, emitting a blinding flare that engulfs the entire frame. As the flare fades, the CookMirror product silhouette emerges slowly from the pure black background, entering the final LOGO reveal.

---

## 🎬 场景 10:LOGO 定版

### 中文

纯黑背景,中央 CookMirror 产品外形从虚化逐渐聚焦,镜面亮起蓝色光晕。镜面正中央慢慢浮现金属质感 LOGO "CookMirror · 厨魔镜",下方一行小字 slogan:"放大每一位厨师"。光晕缓缓扩散后定格。极简优雅,6 秒。
```

### English

Pure black background. CookMirror product silhouette gradually focuses from blur, mirror glows blue. Metallic logo "CookMirror" slowly emerges at center, with tagline beneath: "Amplify Every Cook." Glow softly expands and freezes. Minimalist and elegant, 6 seconds.


---

## 🎨 视觉风格统一指南(全片适用)

| 维度 | 规范 |
|---|---|
| 画面比例 | 16:9 横版 / 可同步出 9:16 竖版用于短视频 |
| 分辨率 | 4K (3840×2160) |
| 帧率 | 24fps 电影感 |
| 色调 | 暖橙为基础 + 科技蓝点缀(参考 Apple 产品片) |
| 镜头语言 | 多用慢推、慢摇、轻微 dolly-in,避免快速剪辑 |
| 景深 | 浅景深突出主体,虚化背景增强电影感 |
| UI 风格 | 极简扁平,半透明毛玻璃质感,微动效 |
| 音效 | 轻柔钢琴 + 环境厨房音(切菜、油爆)+ 关键节点提示音 |
| 字幕 | 思源黑体 / SF Pro,白色或浅金色,带柔和阴影 |

---

## 🛠️ 使用建议

1. **先短后长**:先分别生成 10 个独立场景,挑选效果最好的再拼成完整 90s 短片
2. **多版本对比**:同一场景用 Sora 和可灵各跑一遍,选画质更稳定的版本
3. **参考图喂入**:如有 CookMirror 产品渲染图,优先用作首帧参考(可灵/Pika 支持"图生视频")
4. **角色一致性**:同一人物在不同场景出现时,使用 Sora 的 character reference 功能或可灵的"主体保持"
5. **修复瑕疵**:UI 文字 AI 生成易乱码,后期用 AE/PR 二次合成文字层更稳
6. **配音可分离**:中文/英文双轨配音,海外汇报版可切换

---

## 📦 交付输出建议

| 用途 | 时长 | 输出 |
|---|---|---|
| 课程答辩 PPT 嵌入 | 60-90s | 横版 MP4 1080p |
| 课堂演讲开场片 | 15-30s | 横版精剪版 |
| 课后分享 / 朋友圈 | 30s | 竖版 9:16 |
| 静态海报封面 | - | 抽取场景 1 / 7 的关键帧 |
