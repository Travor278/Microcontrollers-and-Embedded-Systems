/* global React, ReactDOM, RECIPE, fmtSecs, StatusBar, StandbyHome, TempGauge, ARLayer, RecipePanel, VoiceCaption, Scheduler, SafetyOverlay, CamFeed, GestureToast */
const { useState, useEffect, useRef, useCallback } = React;

const TOTAL = RECIPE.steps.length;
const CONN_NOTICE = {
  online: "已切回 WiFi 在线模式,云端菜谱理解与语音对话可用",
  g4: "WiFi 心跳失败,已由 EC600S 4G 备用链路接管",
  offline: "已进入离线模式,AR 步骤、锅温读取与关阀闭环继续本地运行",
};
const URL_PARAMS = new URLSearchParams(window.location.search);
const INITIAL_VIEW = URL_PARAMS.get("view") || "standby";
const INITIAL_CONN = ["online","g4","offline"].includes(URL_PARAMS.get("conn")) ? URL_PARAMS.get("conn") : "online";
const INITIAL_HEAT = URL_PARAMS.get("heat") === "1";

function App(){
  const [clock, setClock] = useState(new Date());
  const [mode, setMode] = useState(INITIAL_VIEW === "standby" ? "standby" : "cooking");
  const [focus, setFocus] = useState(INITIAL_VIEW === "schedule" ? "schedule" : "ar");
  const [step, setStep] = useState(0);
  const [potTemp, setPotTemp] = useState(INITIAL_VIEW === "safety" ? 254 : INITIAL_HEAT ? 238 : 118);
  const [conn, setConn] = useState(INITIAL_CONN);
  const [voiceLive, setVoiceLive] = useState(false);
  const [cap, setCap] = useState({ html:"", show:false });
  const [alert, setAlert] = useState(INITIAL_VIEW === "safety");
  const [count, setCount] = useState(5);
  const [fishSecs, setFishSecs] = useState(96);
  const [scale, setScale] = useState(1);
  const [gesture, setGesture] = useState({ show:false, dir:"next" });
  const [heatBoost, setHeatBoost] = useState(INITIAL_HEAT);

  const capTimer = useRef(null);
  const gestureTimer = useRef(null);
  const fishAnnounced = useRef(false);
  const tempAnnounced = useRef(false);

  useEffect(()=>{
    const id = setInterval(()=>setClock(new Date()),1000);
    return ()=>clearInterval(id);
  },[]);

  useEffect(()=>{
    const fit = ()=>{
      const usableW = Math.max(360, window.innerWidth - 72);
      const usableH = Math.max(260, window.innerHeight - 188);
      const s = Math.min(usableW / 1920, usableH / 1080);
      setScale(Math.max(0.22, s));
    };
    fit();
    window.addEventListener("resize", fit);
    return ()=>window.removeEventListener("resize", fit);
  },[]);

  const showCap = useCallback((html)=>{
    setCap({ html, show:true });
    setVoiceLive(true);
    clearTimeout(capTimer.current);
    capTimer.current = setTimeout(()=>{
      setCap(c=>({ ...c, show:false }));
      setVoiceLive(false);
    }, 4200);
  },[]);

  useEffect(()=>{
    return ()=>{
      clearTimeout(capTimer.current);
      clearTimeout(gestureTimer.current);
    };
  },[]);

  useEffect(()=>{
    if(mode !== "cooking" || alert) return;
    const target = heatBoost ? Math.max(236, RECIPE.steps[step].target) : RECIPE.steps[step].target;
    const id = setInterval(()=>{
      setPotTemp(p=>{
        const drift = target - p;
        const next = p + drift * 0.105 + (Math.random() - 0.5) * 2.4;
        return Math.round(next * 10) / 10;
      });
    }, 180);
    return ()=>clearInterval(id);
  },[mode, step, alert, heatBoost]);

  useEffect(()=>{
    if(!heatBoost) return;
    const id = setTimeout(()=>setHeatBoost(false), 9000);
    return ()=>clearTimeout(id);
  },[heatBoost]);

  useEffect(()=>{
    if(mode !== "cooking" || alert) return;
    const id = setInterval(()=>setFishSecs(s=> s > 0 ? s - 1 : 0), 1000);
    return ()=>clearInterval(id);
  },[mode, alert]);

  useEffect(()=>{
    if(fishSecs === 30 && !fishAnnounced.current){
      fishAnnounced.current = true;
      showCap("清蒸鱼还有 <b>30 秒</b>,建议准备关火取出");
    }
  },[fishSecs, showCap]);

  useEffect(()=>{
    if(mode === "cooking" && !alert){
      showCap(RECIPE.steps[step].voice);
    }
  },[step, mode, alert, showCap]);

  useEffect(()=>{
    if(mode !== "cooking" || alert) return;
    if((potTemp > 220 || heatBoost) && !tempAnnounced.current){
      tempAnnounced.current = true;
      showCap("锅温超过 <b>220℃</b>,系统建议调小火或立即下锅");
    }
    if(potTemp < 205 && !heatBoost && tempAnnounced.current){
      tempAnnounced.current = false;
    }
  },[potTemp, heatBoost, mode, alert, showCap]);

  useEffect(()=>{
    if(!alert || count <= 0) return;
    const id = setTimeout(()=>setCount(c=>c-1), 1000);
    return ()=>clearTimeout(id);
  },[alert, count]);

  const pulseGesture = useCallback((dir)=>{
    setGesture({ show:true, dir });
    clearTimeout(gestureTimer.current);
    gestureTimer.current = setTimeout(()=>setGesture(g=>({ ...g, show:false })), 980);
  },[]);

  const next = useCallback(()=>{
    setStep(s=>Math.min(TOTAL - 1, s + 1));
    pulseGesture("next");
  },[pulseGesture]);

  const prev = useCallback(()=>{
    setStep(s=>Math.max(0, s - 1));
    pulseGesture("prev");
  },[pulseGesture]);

  useEffect(()=>{
    const h = (e)=>{
      if(mode !== "cooking") return;
      if(e.key === "ArrowRight") next();
      if(e.key === "ArrowLeft") prev();
    };
    window.addEventListener("keydown", h);
    return ()=>window.removeEventListener("keydown", h);
  },[mode, next, prev]);

  const startCooking = useCallback(()=>{
    setMode("cooking");
    setFocus("ar");
    setStep(0);
    setPotTemp(132);
    setFishSecs(96);
    setHeatBoost(false);
    fishAnnounced.current = false;
  },[]);

  const setConnection = useCallback((nextConn)=>{
    setConn(nextConn);
    showCap(CONN_NOTICE[nextConn]);
  },[showCap]);

  const cycleConn = useCallback(()=>{
    const order = ["online", "g4", "offline"];
    const nextConn = order[(order.indexOf(conn) + 1) % order.length];
    setConnection(nextConn);
  },[conn, setConnection]);

  const wake = useCallback(()=>{
    if(conn === "offline"){
      showCap("离线唤醒已响应:支持下一步、上一步、锅温与紧急关阀");
      return;
    }
    showCap("我在,你可以说 <b>下一步</b>、<b>火大了吗</b> 或 <b>开始调度</b>");
  },[conn, showCap]);

  const forceHeat = useCallback(()=>{
    setMode("cooking");
    setFocus("ar");
    setHeatBoost(true);
    setPotTemp(238);
    showCap("红外阵列检测到锅心温度快速上升,已进入锅温预警");
  },[showCap]);

  const triggerAlert = useCallback(()=>{
    setMode("cooking");
    setFocus("ar");
    setHeatBoost(false);
    setPotTemp(254);
    setCount(5);
    setAlert(true);
  },[]);

  const clearAlert = useCallback(()=>{
    setAlert(false);
    setCount(5);
    setHeatBoost(false);
    setPotTemp(188);
    showCap("警报已解除,燃气阀保持关闭前请完成现场检查");
  },[showCap]);

  const showStandby = ()=>{
    setMode("standby");
    setFocus("ar");
    setAlert(false);
  };

  const showAr = ()=>{
    setMode("cooking");
    setFocus("ar");
    setAlert(false);
  };

  const showSchedule = ()=>{
    setMode("cooking");
    setFocus("schedule");
    setAlert(false);
  };

  const st = RECIPE.steps[step];
  const isCooking = mode === "cooking";

  return (
    <div className="proto-root">
      <div className="mirror-outer" aria-label="CookMirror 16:9 mirror prototype">
        <div style={{ width:1920 * scale + "px", height:1080 * scale + "px" }}>
          <div className={"mirror-stage conn-" + conn + (alert ? " alerting" : "")} style={{ transform:`scale(${scale})` }}>
            <div className="mirror-scene">
              <CamFeed/>
              <div className="scene-veil"></div>
              <div className="scene-sheen"></div>
              <div className="scanlines"></div>
            </div>

            <StatusBar clock={clock} conn={conn} onCycleConn={cycleConn} voiceLive={voiceLive} onVoice={wake}/>

            {mode === "standby" && <StandbyHome clock={clock} conn={conn} onStart={startCooking}/>}

            {isCooking && (
              <div className={"hud focus-" + focus}>
                <ARLayer step={st} muted={focus === "schedule"}/>
                <TempGauge temp={potTemp} target={st.target} pos={st.tempPos}/>
                <RecipePanel step={st} idx={step} total={TOTAL} timer={fmtSecs(Math.max(0,180 - step * 20))} conn={conn} thermalWarn={potTemp > 220 || heatBoost}/>
                <VoiceCaption html={cap.html} show={cap.show}/>
                <GestureToast gesture={gesture}/>
                <div className="gesture-hint">
                  <span className="gesture-mark"></span>
                  <span>LD2410B 手势就绪</span>
                  <b>{step + 1}/{TOTAL}</b>
                </div>
                <Scheduler activeStep={step} total={TOTAL} fishSecs={fishSecs} focus={focus === "schedule"} onFocus={()=>setFocus("schedule")}/>
              </div>
            )}

            {alert && <SafetyOverlay count={count} onCancel={clearAlert}/>}
          </div>
        </div>
      </div>

      <div className="demo-dock" aria-label="演示控制台">
        <div className="grp">
          <span className="gl">流程</span>
          <button className={"dbtn" + (mode === "standby" ? " on" : "")} onClick={showStandby}>待机镜面</button>
          <button className={"dbtn" + (isCooking && focus === "ar" && !alert ? " on" : "")} onClick={showAr}>AR 引导 HUD</button>
          <button className={"dbtn" + (isCooking && focus === "schedule" && !alert ? " on" : "")} onClick={showSchedule}>多菜调度</button>
          <button className={"dbtn" + (alert ? " on danger" : "")} onClick={triggerAlert}>安全关阀</button>
        </div>
        <div className="grp">
          <span className="gl">隔空手势</span>
          <button className="dbtn" onClick={prev}>上一步</button>
          <button className="dbtn" onClick={next}>下一步</button>
        </div>
        <div className="grp">
          <span className="gl">链路</span>
          <button className={"dbtn" + (conn === "online" ? " on" : "")} onClick={()=>setConnection("online")}>在线</button>
          <button className={"dbtn" + (conn === "g4" ? " on" : "")} onClick={()=>setConnection("g4")}>4G</button>
          <button className={"dbtn" + (conn === "offline" ? " on" : "")} onClick={()=>setConnection("offline")}>离线</button>
        </div>
        <div className="grp">
          <span className="gl">传感器</span>
          <button className="dbtn" onClick={wake}>语音唤醒</button>
          <button className="dbtn warn" onClick={forceHeat}>锅温预警</button>
        </div>
        <span className="demo-hint">CookMirror 厨魔镜 · 16:9 后置 LCD 镜面交互原型</span>
      </div>
    </div>
  );
}

ReactDOM.createRoot(document.getElementById("root")).render(<App/>);
