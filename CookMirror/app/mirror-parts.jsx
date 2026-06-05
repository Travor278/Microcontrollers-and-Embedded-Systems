/* global React */

const RECIPE = {
  name: "西红柿炒蛋",
  steps: [
    {
      title:"热锅倒油",
      detail:"中火热锅,倒入食用油,等待油温升至 180℃。",
      ing:"食用油 15ml",
      target:180,
      arLabel:"油温达 180℃ 即可下锅",
      arPos:{ x:610, y:600 },
      tempPos:{ x:980, y:360 },
      voice:"油温 <b>180℃</b>,可以下锅了",
    },
    {
      title:"倒入蛋液",
      detail:"将打散的蛋液沿锅边缓缓倒入,不要翻动。",
      ing:"鸡蛋 3 个",
      target:165,
      arLabel:"蛋液沿锅边倒入",
      arPos:{ x:640, y:560 },
      tempPos:{ x:980, y:360 },
      voice:"蛋液<b>沿锅边</b>倒入,先别翻动",
    },
    {
      title:"划散盛出",
      detail:"蛋液微凝后用铲快速划散成块,盛出备用。",
      ing:"已完成",
      target:150,
      arLabel:"快速划散成块",
      arPos:{ x:600, y:580 },
      tempPos:{ x:980, y:360 },
      voice:"微凝后<b>快速划散</b>,盛出备用",
    },
    {
      title:"爆香下番茄",
      detail:"余油下姜末爆香,放入番茄块翻炒出汁。",
      ing:"番茄 2 个 · 姜末",
      target:175,
      arLabel:"番茄下锅翻炒",
      arPos:{ x:650, y:600 },
      tempPos:{ x:980, y:360 },
      voice:"番茄下锅,<b>翻炒出汁</b>",
    },
    {
      title:"调味",
      detail:"加盐与少许糖提鲜,翻炒均匀融合。",
      ing:"盐 1g · 糖 2g",
      target:168,
      arLabel:"加盐 1g + 糖 2g",
      arPos:{ x:680, y:560 },
      tempPos:{ x:980, y:360 },
      voice:"今日钠摄入已达 62%,本菜<b>减盐至 1g</b>",
    },
    {
      title:"回锅收汁",
      detail:"倒回蛋块,大火快速翻炒收汁后出锅装盘。",
      ing:"蛋块回锅",
      target:172,
      arLabel:"倒回蛋块收汁",
      arPos:{ x:620, y:580 },
      tempPos:{ x:980, y:360 },
      voice:"倒回蛋块,<b>收汁出锅</b>",
    },
  ],
};

function two(n){ return n < 10 ? "0" + n : "" + n; }
function fmtTime(d){ return { h:two(d.getHours()), m:two(d.getMinutes()), s:two(d.getSeconds()) }; }
function fmtSecs(s){ return Math.floor(s / 60) + ":" + two(s % 60); }
const WK = ["周日","周一","周二","周三","周四","周五","周六"];

function StatusBar({ clock, conn, onCycleConn, voiceLive, onVoice }){
  const t = fmtTime(clock);
  const connMap = {
    online:{ cls:"online", code:"WiFi", label:"在线 · 云端 AI", sub:"MQTTS 心跳 28ms" },
    g4:{ cls:"g4", code:"4G", label:"备用链路", sub:"EC600S 已接管" },
    offline:{ cls:"offline", code:"OFF", label:"离线模式", sub:"安全闭环本地" },
  };
  const c = connMap[conn];
  return (
    <div className="statusbar">
      <div className="sb-clock">
        <div className="sb-time">{t.h}:{t.m}<span className="sec">:{t.s}</span></div>
        <div>
          <div className="sb-date">{clock.getMonth()+1}月{clock.getDate()}日 · {WK[clock.getDay()]}</div>
          <div className="sb-season">立夏 · 宜清淡</div>
        </div>
      </div>
      <div className="sb-env">
        <div className="env-pill"><span className="env-ic"></span>厨房 <b>24℃</b></div>
        <div className="env-pill"><span className="env-ic ok"></span>空气 <b>良好</b></div>
        <div className="env-pill"><span className="env-ic"></span>燃气 <b>正常</b></div>
      </div>
      <div className="sb-right">
        <button className={"voice-orb" + (voiceLive ? " live" : "")} onClick={onVoice} aria-label="语音唤醒">
          <span className="orb"><i></i></span>
          <span className="voice-lbl">{voiceLive ? "聆听中" : "嘿魔镜"}</span>
        </button>
        <button className={"conn " + c.cls} onClick={onCycleConn} title="点击切换网络状态">
          <span className="conn-code">{c.code}</span>
          <span><b>{c.label}</b><small>{c.sub}</small></span>
        </button>
      </div>
    </div>
  );
}

function StandbyHome({ clock, conn, onStart }){
  const t = fmtTime(clock);
  const connText = conn === "offline" ? "离线缓存菜谱可用" : conn === "g4" ? "4G 备用链路在线" : "云端营养模型在线";
  return (
    <div className="standby">
      <div className="sb-hero">
        <div className="standby-copy">
          <div className="brand-line">CookMirror 厨魔镜</div>
          <div className="sb-greet">晚上好,<b>金奕帆</b></div>
          <div className="sb-bigclock">{t.h}<span className="colon">:</span>{t.m}</div>
          <div className="standby-strip">
            <span>{connText}</span>
            <span>番茄还剩 2 天</span>
            <span>低钠推荐已启用</span>
          </div>
        </div>
        <div className="mirror-you" aria-hidden="true">
          <div className="reflection-head"></div>
          <div className="reflection-body"></div>
          <div className="reflection-line a"></div>
          <div className="reflection-line b"></div>
          <div className="reflection-line c"></div>
        </div>
      </div>

      <div className="sb-cards">
        <button className="glass glow-cy reco" onClick={onStart}>
          <span className="reco-tag">临期食材优先 · 15 分钟完成</span>
          <h3>今晚推荐:西红柿炒蛋</h3>
          <p>消耗临期番茄与鸡蛋 · 适合慢病低钠 · 6 步 AR 引导</p>
          <span className="reco-cta">开始烹饪</span>
        </button>
        <div className="glass amb">
          <div className="l">本周营养</div>
          <div className="v">82<small>/100</small></div>
          <div className="s">蛋白质达标 · 钠偏高,注意减盐</div>
        </div>
        <div className="glass amb">
          <div className="l">冰箱临期</div>
          <div className="v">3<small> 样</small></div>
          <div className="s">番茄 2天 · 鸡蛋 4天 · 牛奶 3天</div>
        </div>
      </div>
    </div>
  );
}

function TempGauge({ temp, target, pos }){
  const hot = temp > 220;
  const ready = Math.abs(temp - target) < 8 && !hot;
  const pct = Math.max(0, Math.min(100, ((temp - 20) / (280 - 20)) * 100));
  return (
    <div className="temp-gauge" style={{ left:pos.x + "px", top:pos.y + "px" }}>
      <div className="glass glow-cy tg-card">
        <div className="tg-lbl">红外锅温 · MLX90640</div>
        <div className={"tg-val" + (hot ? " hot" : "")}>{Math.round(temp)}<small>℃</small></div>
        <div className="thermal-map" aria-hidden="true">
          <i className="cell c1"></i><i className="cell c2"></i><i className="cell c3"></i>
          <i className="cell c4"></i><i className="cell c5"></i><i className="cell c6"></i>
        </div>
        <div className="tg-bar"><span className="mark" style={{ left:pct + "%" }}></span></div>
        <div className={"tg-status" + (hot ? " hot" : ready ? " ready" : "")}>
          {hot ? "过高 · 即将冒烟" : ready ? "达到目标油温" : temp < target ? "升温中" : "保持火候"}
        </div>
      </div>
    </div>
  );
}

function ARLayer({ step, muted }){
  return (
    <React.Fragment>
      <div className={"ar-target" + (muted ? " muted" : "")} style={{ left:step.arPos.x + "px", top:step.arPos.y + "px" }}>
        <div className="ar-ring"></div>
        <div className="ar-ring r2"></div>
        <div className="ar-dot"></div>
      </div>
      <div className={"ar-arrow" + (muted ? " muted" : "")} style={{ left:step.arPos.x + "px", top:(step.arPos.y - 150) + "px" }}>
        <div className="lbl">{step.arLabel}</div>
        <div className="stem"></div>
      </div>
    </React.Fragment>
  );
}

function RecipePanel({ step, idx, total, timer, conn, thermalWarn }){
  const connLabel = conn === "offline" ? "离线缓存" : conn === "g4" ? "4G 同步" : "云端协同";
  return (
    <div className={"glass glow-cy recipe" + (thermalWarn ? " temp-alert" : "")}>
      <div className="recipe-top">
        <div>
          <div className="recipe-name">{RECIPE.name}</div>
          <div className="recipe-mode">{connLabel} · STM32 锅温闭环</div>
        </div>
        <div className="step-badge">步骤 {idx+1} / {total}</div>
      </div>
      <div className="step-prog">
        {RECIPE.steps.map((_,i)=>(
          <i key={i} className={i < idx ? "done" : i === idx ? "cur" : ""}></i>
        ))}
      </div>
      <div className="step-instr">{step.title}</div>
      <div className="step-detail">{step.detail}</div>
      <div className="step-ing">
        <div className="ing-chip"><span className="d"></span>{step.ing}</div>
        <div className="step-timer">计时 {timer}</div>
      </div>
      {thermalWarn && <div className="inline-warn">锅温超过安全阈值,建议降火或立即下锅</div>}
    </div>
  );
}

function VoiceCaption({ html, show }){
  return (
    <div className={"glass voicecap" + (show ? "" : " hide")}>
      <div className="wave" aria-hidden="true">
        <i style={{ height:"14px" }}></i><i></i><i></i><i></i><i></i><i></i>
      </div>
      <div className="cap" dangerouslySetInnerHTML={{ __html:html }}></div>
    </div>
  );
}

function GestureToast({ gesture }){
  return (
    <div className={"gesture-toast " + gesture.dir + (gesture.show ? " show" : "")}>
      <span className="gesture-sweep"></span>
      <b>{gesture.dir === "next" ? "右向左挥手" : "左向右挥手"}</b>
      <small>{gesture.dir === "next" ? "进入下一步" : "返回上一步"}</small>
    </div>
  );
}

function Scheduler({ activeStep, total, fishSecs, focus, onFocus }){
  const actPct = Math.round(((activeStep + 1) / total) * 100);
  const fishAlert = fishSecs <= 30;
  const fishPct = Math.max(4, Math.min(100, 100 - (fishSecs / 300) * 100));
  return (
    <button className={"glass scheduler" + (focus ? " focus" : "")} onClick={onFocus}>
      <div className="sch-head">
        <div className="sch-title"><span className="k">AI 调度管家</span> 三菜一汤 · 并行进行中</div>
        <div className="sch-legend">
          <span className="dot-act">进行中</span>
          <span className="dot-run">加热中</span>
          <span className="dot-wait">待开始</span>
        </div>
      </div>
      <DishRow state="act" name="西红柿炒蛋" pct={actPct} meta={<span>步骤 <b>{activeStep+1}/{total}</b> · 进行中</span>}/>
      <DishRow state="run" name="清蒸鱼" pct={fishPct} alert={fishAlert} meta={fishAlert ? <span>还有 <b>{fmtSecs(fishSecs)}</b> 请关火取出</span> : <span>蒸制中 · 还有 <b>{fmtSecs(fishSecs)}</b></span>}/>
      <DishRow state="wait" name="紫菜蛋汤" pct={0} meta={<span>待开始 · 炒蛋后接手</span>}/>
      {focus && (
        <div className="parallel-grid">
          <div><b>火力建议</b><span>炒蛋维持中火,鱼锅 90 秒后关火,汤锅延后 4 分钟点火。</span></div>
          <div><b>冲突避让</b><span>下一次投料与蒸鱼取出相差 38 秒,镜面将优先提醒蒸鱼。</span></div>
          <div><b>边云协同</b><span>断网时保留本地时间轴与锅温告警,云端只影响自然语言问答。</span></div>
        </div>
      )}
    </button>
  );
}

function DishRow({ state, name, pct, meta, alert }){
  return (
    <div className="dish-row">
      <div className="dish-name"><span className={"st " + state}></span>{name}</div>
      <div className="dish-track"><div className={"dish-fill " + state} style={{ width:pct + "%" }}></div></div>
      <div className={"dish-meta" + (alert ? " alert" : "")}>{meta}</div>
    </div>
  );
}

function SafetyOverlay({ count, onCancel }){
  const sealed = count <= 0;
  return (
    <div className={"safety" + (sealed ? " sealed" : "")}>
      <div className="saf-mark"><span></span></div>
      <div className="saf-title">{sealed ? "燃气阀已关闭" : "检测到燃气泄漏"}</div>
      <div className="saf-sub">MQ-5 + VOC + 火焰图像 · 三重确认</div>
      <div className="saf-count">{sealed ? "PC0" : count}</div>
      <div className="saf-act">{sealed ? "继电器吸合 · DN15 常闭阀已断气" : "即将自动关闭燃气电磁阀"}</div>
      <div className="saf-sensors">
        <div className="saf-sensor"><div className="l">MQ-5 燃气</div><div className="v">1820<span>ppm</span></div></div>
        <div className="saf-sensor"><div className="l">VOC</div><div className="v">超阈值</div></div>
        <div className="saf-sensor"><div className="l">响应延迟</div><div className="v">87<span>ms</span></div></div>
        <div className="saf-sensor"><div className="l">执行</div><div className="v">PC0 关阀</div></div>
      </div>
      <button className="saf-cancel" onClick={onCancel}>我已处理 · 解除警报</button>
      <div className="saf-note">安全闭环完全在 STM32 本地运行 · 断网仍可靠关阀</div>
    </div>
  );
}

function CamFeed(){
  return (
    <div className="cam-feed" aria-hidden="true">
      <div className="wall-grid"></div>
      <div className="counter"></div>
      <div className="cooktop">
        <span className="burner b1"></span>
        <span className="burner b2"></span>
        <span className="burner b3"></span>
      </div>
      <div className="pan">
        <span className="pan-core"></span>
        <span className="steam s1"></span>
        <span className="steam s2"></span>
        <span className="steam s3"></span>
      </div>
      <div className="ingredient plate-a"></div>
      <div className="ingredient plate-b"></div>
      <div className="knife-line"></div>
    </div>
  );
}

Object.assign(window, {
  RECIPE, fmtSecs, fmtTime,
  StatusBar, StandbyHome, TempGauge, ARLayer, RecipePanel,
  VoiceCaption, Scheduler, SafetyOverlay, CamFeed, GestureToast,
});
