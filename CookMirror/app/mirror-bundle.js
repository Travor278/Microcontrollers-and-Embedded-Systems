/* Generated from mirror-parts.jsx + mirror-app.jsx for direct file opening. */
"use strict";

function _typeof(o) { "@babel/helpers - typeof"; return _typeof = "function" == typeof Symbol && "symbol" == typeof Symbol.iterator ? function (o) { return typeof o; } : function (o) { return o && "function" == typeof Symbol && o.constructor === Symbol && o !== Symbol.prototype ? "symbol" : typeof o; }, _typeof(o); }
function ownKeys(e, r) { var t = Object.keys(e); if (Object.getOwnPropertySymbols) { var o = Object.getOwnPropertySymbols(e); r && (o = o.filter(function (r) { return Object.getOwnPropertyDescriptor(e, r).enumerable; })), t.push.apply(t, o); } return t; }
function _objectSpread(e) { for (var r = 1; r < arguments.length; r++) { var t = null != arguments[r] ? arguments[r] : {}; r % 2 ? ownKeys(Object(t), !0).forEach(function (r) { _defineProperty(e, r, t[r]); }) : Object.getOwnPropertyDescriptors ? Object.defineProperties(e, Object.getOwnPropertyDescriptors(t)) : ownKeys(Object(t)).forEach(function (r) { Object.defineProperty(e, r, Object.getOwnPropertyDescriptor(t, r)); }); } return e; }
function _defineProperty(e, r, t) { return (r = _toPropertyKey(r)) in e ? Object.defineProperty(e, r, { value: t, enumerable: !0, configurable: !0, writable: !0 }) : e[r] = t, e; }
function _toPropertyKey(t) { var i = _toPrimitive(t, "string"); return "symbol" == _typeof(i) ? i : i + ""; }
function _toPrimitive(t, r) { if ("object" != _typeof(t) || !t) return t; var e = t[Symbol.toPrimitive]; if (void 0 !== e) { var i = e.call(t, r || "default"); if ("object" != _typeof(i)) return i; throw new TypeError("@@toPrimitive must return a primitive value."); } return ("string" === r ? String : Number)(t); }
function _slicedToArray(r, e) { return _arrayWithHoles(r) || _iterableToArrayLimit(r, e) || _unsupportedIterableToArray(r, e) || _nonIterableRest(); }
function _nonIterableRest() { throw new TypeError("Invalid attempt to destructure non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); }
function _unsupportedIterableToArray(r, a) { if (r) { if ("string" == typeof r) return _arrayLikeToArray(r, a); var t = {}.toString.call(r).slice(8, -1); return "Object" === t && r.constructor && (t = r.constructor.name), "Map" === t || "Set" === t ? Array.from(r) : "Arguments" === t || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(t) ? _arrayLikeToArray(r, a) : void 0; } }
function _arrayLikeToArray(r, a) { (null == a || a > r.length) && (a = r.length); for (var e = 0, n = Array(a); e < a; e++) n[e] = r[e]; return n; }
function _iterableToArrayLimit(r, l) { var t = null == r ? null : "undefined" != typeof Symbol && r[Symbol.iterator] || r["@@iterator"]; if (null != t) { var e, n, i, u, a = [], f = !0, o = !1; try { if (i = (t = t.call(r)).next, 0 === l) { if (Object(t) !== t) return; f = !1; } else for (; !(f = (e = i.call(t)).done) && (a.push(e.value), a.length !== l); f = !0); } catch (r) { o = !0, n = r; } finally { try { if (!f && null != t["return"] && (u = t["return"](), Object(u) !== u)) return; } finally { if (o) throw n; } } return a; } }
function _arrayWithHoles(r) { if (Array.isArray(r)) return r; }
/* global React */

var RECIPE = {
  name: "西红柿炒蛋",
  steps: [{
    title: "热锅倒油",
    detail: "中火热锅,倒入食用油,等待油温升至 180℃。",
    ing: "食用油 15ml",
    target: 180,
    arLabel: "油温达 180℃ 即可下锅",
    arPos: {
      x: 610,
      y: 600
    },
    tempPos: {
      x: 980,
      y: 360
    },
    voice: "油温 <b>180℃</b>,可以下锅了"
  }, {
    title: "倒入蛋液",
    detail: "将打散的蛋液沿锅边缓缓倒入,不要翻动。",
    ing: "鸡蛋 3 个",
    target: 165,
    arLabel: "蛋液沿锅边倒入",
    arPos: {
      x: 640,
      y: 560
    },
    tempPos: {
      x: 980,
      y: 360
    },
    voice: "蛋液<b>沿锅边</b>倒入,先别翻动"
  }, {
    title: "划散盛出",
    detail: "蛋液微凝后用铲快速划散成块,盛出备用。",
    ing: "已完成",
    target: 150,
    arLabel: "快速划散成块",
    arPos: {
      x: 600,
      y: 580
    },
    tempPos: {
      x: 980,
      y: 360
    },
    voice: "微凝后<b>快速划散</b>,盛出备用"
  }, {
    title: "爆香下番茄",
    detail: "余油下姜末爆香,放入番茄块翻炒出汁。",
    ing: "番茄 2 个 · 姜末",
    target: 175,
    arLabel: "番茄下锅翻炒",
    arPos: {
      x: 650,
      y: 600
    },
    tempPos: {
      x: 980,
      y: 360
    },
    voice: "番茄下锅,<b>翻炒出汁</b>"
  }, {
    title: "调味",
    detail: "加盐与少许糖提鲜,翻炒均匀融合。",
    ing: "盐 1g · 糖 2g",
    target: 168,
    arLabel: "加盐 1g + 糖 2g",
    arPos: {
      x: 680,
      y: 560
    },
    tempPos: {
      x: 980,
      y: 360
    },
    voice: "今日钠摄入已达 62%,本菜<b>减盐至 1g</b>"
  }, {
    title: "回锅收汁",
    detail: "倒回蛋块,大火快速翻炒收汁后出锅装盘。",
    ing: "蛋块回锅",
    target: 172,
    arLabel: "倒回蛋块收汁",
    arPos: {
      x: 620,
      y: 580
    },
    tempPos: {
      x: 980,
      y: 360
    },
    voice: "倒回蛋块,<b>收汁出锅</b>"
  }]
};
function two(n) {
  return n < 10 ? "0" + n : "" + n;
}
function fmtTime(d) {
  return {
    h: two(d.getHours()),
    m: two(d.getMinutes()),
    s: two(d.getSeconds())
  };
}
function fmtSecs(s) {
  return Math.floor(s / 60) + ":" + two(s % 60);
}
var WK = ["周日", "周一", "周二", "周三", "周四", "周五", "周六"];
function StatusBar(_ref) {
  var clock = _ref.clock,
    conn = _ref.conn,
    onCycleConn = _ref.onCycleConn,
    voiceLive = _ref.voiceLive,
    onVoice = _ref.onVoice;
  var t = fmtTime(clock);
  var connMap = {
    online: {
      cls: "online",
      code: "WiFi",
      label: "在线 · 云端 AI",
      sub: "MQTTS 心跳 28ms"
    },
    g4: {
      cls: "g4",
      code: "4G",
      label: "备用链路",
      sub: "EC600S 已接管"
    },
    offline: {
      cls: "offline",
      code: "OFF",
      label: "离线模式",
      sub: "安全闭环本地"
    }
  };
  var c = connMap[conn];
  return /*#__PURE__*/React.createElement("div", {
    className: "statusbar"
  }, /*#__PURE__*/React.createElement("div", {
    className: "sb-clock"
  }, /*#__PURE__*/React.createElement("div", {
    className: "sb-time"
  }, t.h, ":", t.m, /*#__PURE__*/React.createElement("span", {
    className: "sec"
  }, ":", t.s)), /*#__PURE__*/React.createElement("div", null, /*#__PURE__*/React.createElement("div", {
    className: "sb-date"
  }, clock.getMonth() + 1, "\u6708", clock.getDate(), "\u65E5 \xB7 ", WK[clock.getDay()]), /*#__PURE__*/React.createElement("div", {
    className: "sb-season"
  }, "\u7ACB\u590F \xB7 \u5B9C\u6E05\u6DE1"))), /*#__PURE__*/React.createElement("div", {
    className: "sb-env"
  }, /*#__PURE__*/React.createElement("div", {
    className: "env-pill"
  }, /*#__PURE__*/React.createElement("span", {
    className: "env-ic"
  }), "\u53A8\u623F ", /*#__PURE__*/React.createElement("b", null, "24\u2103")), /*#__PURE__*/React.createElement("div", {
    className: "env-pill"
  }, /*#__PURE__*/React.createElement("span", {
    className: "env-ic ok"
  }), "\u7A7A\u6C14 ", /*#__PURE__*/React.createElement("b", null, "\u826F\u597D")), /*#__PURE__*/React.createElement("div", {
    className: "env-pill"
  }, /*#__PURE__*/React.createElement("span", {
    className: "env-ic"
  }), "\u71C3\u6C14 ", /*#__PURE__*/React.createElement("b", null, "\u6B63\u5E38"))), /*#__PURE__*/React.createElement("div", {
    className: "sb-right"
  }, /*#__PURE__*/React.createElement("button", {
    className: "voice-orb" + (voiceLive ? " live" : ""),
    onClick: onVoice,
    "aria-label": "\u8BED\u97F3\u5524\u9192"
  }, /*#__PURE__*/React.createElement("span", {
    className: "orb"
  }, /*#__PURE__*/React.createElement("i", null)), /*#__PURE__*/React.createElement("span", {
    className: "voice-lbl"
  }, voiceLive ? "聆听中" : "嘿魔镜")), /*#__PURE__*/React.createElement("button", {
    className: "conn " + c.cls,
    onClick: onCycleConn,
    title: "\u70B9\u51FB\u5207\u6362\u7F51\u7EDC\u72B6\u6001"
  }, /*#__PURE__*/React.createElement("span", {
    className: "conn-code"
  }, c.code), /*#__PURE__*/React.createElement("span", null, /*#__PURE__*/React.createElement("b", null, c.label), /*#__PURE__*/React.createElement("small", null, c.sub)))));
}
function StandbyHome(_ref2) {
  var clock = _ref2.clock,
    conn = _ref2.conn,
    onStart = _ref2.onStart;
  var t = fmtTime(clock);
  var connText = conn === "offline" ? "离线缓存菜谱可用" : conn === "g4" ? "4G 备用链路在线" : "云端营养模型在线";
  return /*#__PURE__*/React.createElement("div", {
    className: "standby"
  }, /*#__PURE__*/React.createElement("div", {
    className: "sb-hero"
  }, /*#__PURE__*/React.createElement("div", {
    className: "standby-copy"
  }, /*#__PURE__*/React.createElement("div", {
    className: "brand-line"
  }, "CookMirror \u53A8\u9B54\u955C"), /*#__PURE__*/React.createElement("div", {
    className: "sb-greet"
  }, "\u665A\u4E0A\u597D,", /*#__PURE__*/React.createElement("b", null, "\u91D1\u5955\u5E06")), /*#__PURE__*/React.createElement("div", {
    className: "sb-bigclock"
  }, t.h, /*#__PURE__*/React.createElement("span", {
    className: "colon"
  }, ":"), t.m), /*#__PURE__*/React.createElement("div", {
    className: "standby-strip"
  }, /*#__PURE__*/React.createElement("span", null, connText), /*#__PURE__*/React.createElement("span", null, "\u756A\u8304\u8FD8\u5269 2 \u5929"), /*#__PURE__*/React.createElement("span", null, "\u4F4E\u94A0\u63A8\u8350\u5DF2\u542F\u7528"))), /*#__PURE__*/React.createElement("div", {
    className: "mirror-you",
    "aria-hidden": "true"
  }, /*#__PURE__*/React.createElement("div", {
    className: "reflection-head"
  }), /*#__PURE__*/React.createElement("div", {
    className: "reflection-body"
  }), /*#__PURE__*/React.createElement("div", {
    className: "reflection-line a"
  }), /*#__PURE__*/React.createElement("div", {
    className: "reflection-line b"
  }), /*#__PURE__*/React.createElement("div", {
    className: "reflection-line c"
  }))), /*#__PURE__*/React.createElement("div", {
    className: "sb-cards"
  }, /*#__PURE__*/React.createElement("button", {
    className: "glass glow-cy reco",
    onClick: onStart
  }, /*#__PURE__*/React.createElement("span", {
    className: "reco-tag"
  }, "\u4E34\u671F\u98DF\u6750\u4F18\u5148 \xB7 15 \u5206\u949F\u5B8C\u6210"), /*#__PURE__*/React.createElement("h3", null, "\u4ECA\u665A\u63A8\u8350:\u897F\u7EA2\u67FF\u7092\u86CB"), /*#__PURE__*/React.createElement("p", null, "\u6D88\u8017\u4E34\u671F\u756A\u8304\u4E0E\u9E21\u86CB \xB7 \u9002\u5408\u6162\u75C5\u4F4E\u94A0 \xB7 6 \u6B65 AR \u5F15\u5BFC"), /*#__PURE__*/React.createElement("span", {
    className: "reco-cta"
  }, "\u5F00\u59CB\u70F9\u996A")), /*#__PURE__*/React.createElement("div", {
    className: "glass amb"
  }, /*#__PURE__*/React.createElement("div", {
    className: "l"
  }, "\u672C\u5468\u8425\u517B"), /*#__PURE__*/React.createElement("div", {
    className: "v"
  }, "82", /*#__PURE__*/React.createElement("small", null, "/100")), /*#__PURE__*/React.createElement("div", {
    className: "s"
  }, "\u86CB\u767D\u8D28\u8FBE\u6807 \xB7 \u94A0\u504F\u9AD8,\u6CE8\u610F\u51CF\u76D0")), /*#__PURE__*/React.createElement("div", {
    className: "glass amb"
  }, /*#__PURE__*/React.createElement("div", {
    className: "l"
  }, "\u51B0\u7BB1\u4E34\u671F"), /*#__PURE__*/React.createElement("div", {
    className: "v"
  }, "3", /*#__PURE__*/React.createElement("small", null, " \u6837")), /*#__PURE__*/React.createElement("div", {
    className: "s"
  }, "\u756A\u8304 2\u5929 \xB7 \u9E21\u86CB 4\u5929 \xB7 \u725B\u5976 3\u5929"))));
}
function TempGauge(_ref3) {
  var temp = _ref3.temp,
    target = _ref3.target,
    pos = _ref3.pos;
  var hot = temp > 220;
  var ready = Math.abs(temp - target) < 8 && !hot;
  var pct = Math.max(0, Math.min(100, (temp - 20) / (280 - 20) * 100));
  return /*#__PURE__*/React.createElement("div", {
    className: "temp-gauge",
    style: {
      left: pos.x + "px",
      top: pos.y + "px"
    }
  }, /*#__PURE__*/React.createElement("div", {
    className: "glass glow-cy tg-card"
  }, /*#__PURE__*/React.createElement("div", {
    className: "tg-lbl"
  }, "\u7EA2\u5916\u9505\u6E29 \xB7 MLX90640"), /*#__PURE__*/React.createElement("div", {
    className: "tg-val" + (hot ? " hot" : "")
  }, Math.round(temp), /*#__PURE__*/React.createElement("small", null, "\u2103")), /*#__PURE__*/React.createElement("div", {
    className: "thermal-map",
    "aria-hidden": "true"
  }, /*#__PURE__*/React.createElement("i", {
    className: "cell c1"
  }), /*#__PURE__*/React.createElement("i", {
    className: "cell c2"
  }), /*#__PURE__*/React.createElement("i", {
    className: "cell c3"
  }), /*#__PURE__*/React.createElement("i", {
    className: "cell c4"
  }), /*#__PURE__*/React.createElement("i", {
    className: "cell c5"
  }), /*#__PURE__*/React.createElement("i", {
    className: "cell c6"
  })), /*#__PURE__*/React.createElement("div", {
    className: "tg-bar"
  }, /*#__PURE__*/React.createElement("span", {
    className: "mark",
    style: {
      left: pct + "%"
    }
  })), /*#__PURE__*/React.createElement("div", {
    className: "tg-status" + (hot ? " hot" : ready ? " ready" : "")
  }, hot ? "过高 · 即将冒烟" : ready ? "达到目标油温" : temp < target ? "升温中" : "保持火候")));
}
function ARLayer(_ref4) {
  var step = _ref4.step,
    muted = _ref4.muted;
  return /*#__PURE__*/React.createElement(React.Fragment, null, /*#__PURE__*/React.createElement("div", {
    className: "ar-target" + (muted ? " muted" : ""),
    style: {
      left: step.arPos.x + "px",
      top: step.arPos.y + "px"
    }
  }, /*#__PURE__*/React.createElement("div", {
    className: "ar-ring"
  }), /*#__PURE__*/React.createElement("div", {
    className: "ar-ring r2"
  }), /*#__PURE__*/React.createElement("div", {
    className: "ar-dot"
  })), /*#__PURE__*/React.createElement("div", {
    className: "ar-arrow" + (muted ? " muted" : ""),
    style: {
      left: step.arPos.x + "px",
      top: step.arPos.y - 150 + "px"
    }
  }, /*#__PURE__*/React.createElement("div", {
    className: "lbl"
  }, step.arLabel), /*#__PURE__*/React.createElement("div", {
    className: "stem"
  })));
}
function RecipePanel(_ref5) {
  var step = _ref5.step,
    idx = _ref5.idx,
    total = _ref5.total,
    timer = _ref5.timer,
    conn = _ref5.conn,
    thermalWarn = _ref5.thermalWarn;
  var connLabel = conn === "offline" ? "离线缓存" : conn === "g4" ? "4G 同步" : "云端协同";
  return /*#__PURE__*/React.createElement("div", {
    className: "glass glow-cy recipe" + (thermalWarn ? " temp-alert" : "")
  }, /*#__PURE__*/React.createElement("div", {
    className: "recipe-top"
  }, /*#__PURE__*/React.createElement("div", null, /*#__PURE__*/React.createElement("div", {
    className: "recipe-name"
  }, RECIPE.name), /*#__PURE__*/React.createElement("div", {
    className: "recipe-mode"
  }, connLabel, " \xB7 STM32 \u9505\u6E29\u95ED\u73AF")), /*#__PURE__*/React.createElement("div", {
    className: "step-badge"
  }, "\u6B65\u9AA4 ", idx + 1, " / ", total)), /*#__PURE__*/React.createElement("div", {
    className: "step-prog"
  }, RECIPE.steps.map(function (_, i) {
    return /*#__PURE__*/React.createElement("i", {
      key: i,
      className: i < idx ? "done" : i === idx ? "cur" : ""
    });
  })), /*#__PURE__*/React.createElement("div", {
    className: "step-instr"
  }, step.title), /*#__PURE__*/React.createElement("div", {
    className: "step-detail"
  }, step.detail), /*#__PURE__*/React.createElement("div", {
    className: "step-ing"
  }, /*#__PURE__*/React.createElement("div", {
    className: "ing-chip"
  }, /*#__PURE__*/React.createElement("span", {
    className: "d"
  }), step.ing), /*#__PURE__*/React.createElement("div", {
    className: "step-timer"
  }, "\u8BA1\u65F6 ", timer)), thermalWarn && /*#__PURE__*/React.createElement("div", {
    className: "inline-warn"
  }, "\u9505\u6E29\u8D85\u8FC7\u5B89\u5168\u9608\u503C,\u5EFA\u8BAE\u964D\u706B\u6216\u7ACB\u5373\u4E0B\u9505"));
}
function VoiceCaption(_ref6) {
  var html = _ref6.html,
    show = _ref6.show;
  return /*#__PURE__*/React.createElement("div", {
    className: "glass voicecap" + (show ? "" : " hide")
  }, /*#__PURE__*/React.createElement("div", {
    className: "wave",
    "aria-hidden": "true"
  }, /*#__PURE__*/React.createElement("i", {
    style: {
      height: "14px"
    }
  }), /*#__PURE__*/React.createElement("i", null), /*#__PURE__*/React.createElement("i", null), /*#__PURE__*/React.createElement("i", null), /*#__PURE__*/React.createElement("i", null), /*#__PURE__*/React.createElement("i", null)), /*#__PURE__*/React.createElement("div", {
    className: "cap",
    dangerouslySetInnerHTML: {
      __html: html
    }
  }));
}
function GestureToast(_ref7) {
  var gesture = _ref7.gesture;
  return /*#__PURE__*/React.createElement("div", {
    className: "gesture-toast " + gesture.dir + (gesture.show ? " show" : "")
  }, /*#__PURE__*/React.createElement("span", {
    className: "gesture-sweep"
  }), /*#__PURE__*/React.createElement("b", null, gesture.dir === "next" ? "右向左挥手" : "左向右挥手"), /*#__PURE__*/React.createElement("small", null, gesture.dir === "next" ? "进入下一步" : "返回上一步"));
}
function Scheduler(_ref8) {
  var activeStep = _ref8.activeStep,
    total = _ref8.total,
    fishSecs = _ref8.fishSecs,
    focus = _ref8.focus,
    onFocus = _ref8.onFocus;
  var actPct = Math.round((activeStep + 1) / total * 100);
  var fishAlert = fishSecs <= 30;
  var fishPct = Math.max(4, Math.min(100, 100 - fishSecs / 300 * 100));
  return /*#__PURE__*/React.createElement("button", {
    className: "glass scheduler" + (focus ? " focus" : ""),
    onClick: onFocus
  }, /*#__PURE__*/React.createElement("div", {
    className: "sch-head"
  }, /*#__PURE__*/React.createElement("div", {
    className: "sch-title"
  }, /*#__PURE__*/React.createElement("span", {
    className: "k"
  }, "AI \u8C03\u5EA6\u7BA1\u5BB6"), " \u4E09\u83DC\u4E00\u6C64 \xB7 \u5E76\u884C\u8FDB\u884C\u4E2D"), /*#__PURE__*/React.createElement("div", {
    className: "sch-legend"
  }, /*#__PURE__*/React.createElement("span", {
    className: "dot-act"
  }, "\u8FDB\u884C\u4E2D"), /*#__PURE__*/React.createElement("span", {
    className: "dot-run"
  }, "\u52A0\u70ED\u4E2D"), /*#__PURE__*/React.createElement("span", {
    className: "dot-wait"
  }, "\u5F85\u5F00\u59CB"))), /*#__PURE__*/React.createElement(DishRow, {
    state: "act",
    name: "\u897F\u7EA2\u67FF\u7092\u86CB",
    pct: actPct,
    meta: /*#__PURE__*/React.createElement("span", null, "\u6B65\u9AA4 ", /*#__PURE__*/React.createElement("b", null, activeStep + 1, "/", total), " \xB7 \u8FDB\u884C\u4E2D")
  }), /*#__PURE__*/React.createElement(DishRow, {
    state: "run",
    name: "\u6E05\u84B8\u9C7C",
    pct: fishPct,
    alert: fishAlert,
    meta: fishAlert ? /*#__PURE__*/React.createElement("span", null, "\u8FD8\u6709 ", /*#__PURE__*/React.createElement("b", null, fmtSecs(fishSecs)), " \u8BF7\u5173\u706B\u53D6\u51FA") : /*#__PURE__*/React.createElement("span", null, "\u84B8\u5236\u4E2D \xB7 \u8FD8\u6709 ", /*#__PURE__*/React.createElement("b", null, fmtSecs(fishSecs)))
  }), /*#__PURE__*/React.createElement(DishRow, {
    state: "wait",
    name: "\u7D2B\u83DC\u86CB\u6C64",
    pct: 0,
    meta: /*#__PURE__*/React.createElement("span", null, "\u5F85\u5F00\u59CB \xB7 \u7092\u86CB\u540E\u63A5\u624B")
  }), focus && /*#__PURE__*/React.createElement("div", {
    className: "parallel-grid"
  }, /*#__PURE__*/React.createElement("div", null, /*#__PURE__*/React.createElement("b", null, "\u706B\u529B\u5EFA\u8BAE"), /*#__PURE__*/React.createElement("span", null, "\u7092\u86CB\u7EF4\u6301\u4E2D\u706B,\u9C7C\u9505 90 \u79D2\u540E\u5173\u706B,\u6C64\u9505\u5EF6\u540E 4 \u5206\u949F\u70B9\u706B\u3002")), /*#__PURE__*/React.createElement("div", null, /*#__PURE__*/React.createElement("b", null, "\u51B2\u7A81\u907F\u8BA9"), /*#__PURE__*/React.createElement("span", null, "\u4E0B\u4E00\u6B21\u6295\u6599\u4E0E\u84B8\u9C7C\u53D6\u51FA\u76F8\u5DEE 38 \u79D2,\u955C\u9762\u5C06\u4F18\u5148\u63D0\u9192\u84B8\u9C7C\u3002")), /*#__PURE__*/React.createElement("div", null, /*#__PURE__*/React.createElement("b", null, "\u8FB9\u4E91\u534F\u540C"), /*#__PURE__*/React.createElement("span", null, "\u65AD\u7F51\u65F6\u4FDD\u7559\u672C\u5730\u65F6\u95F4\u8F74\u4E0E\u9505\u6E29\u544A\u8B66,\u4E91\u7AEF\u53EA\u5F71\u54CD\u81EA\u7136\u8BED\u8A00\u95EE\u7B54\u3002"))));
}
function DishRow(_ref9) {
  var state = _ref9.state,
    name = _ref9.name,
    pct = _ref9.pct,
    meta = _ref9.meta,
    alert = _ref9.alert;
  return /*#__PURE__*/React.createElement("div", {
    className: "dish-row"
  }, /*#__PURE__*/React.createElement("div", {
    className: "dish-name"
  }, /*#__PURE__*/React.createElement("span", {
    className: "st " + state
  }), name), /*#__PURE__*/React.createElement("div", {
    className: "dish-track"
  }, /*#__PURE__*/React.createElement("div", {
    className: "dish-fill " + state,
    style: {
      width: pct + "%"
    }
  })), /*#__PURE__*/React.createElement("div", {
    className: "dish-meta" + (alert ? " alert" : "")
  }, meta));
}
function SafetyOverlay(_ref0) {
  var count = _ref0.count,
    onCancel = _ref0.onCancel;
  var sealed = count <= 0;
  return /*#__PURE__*/React.createElement("div", {
    className: "safety" + (sealed ? " sealed" : "")
  }, /*#__PURE__*/React.createElement("div", {
    className: "saf-mark"
  }, /*#__PURE__*/React.createElement("span", null)), /*#__PURE__*/React.createElement("div", {
    className: "saf-title"
  }, sealed ? "燃气阀已关闭" : "检测到燃气泄漏"), /*#__PURE__*/React.createElement("div", {
    className: "saf-sub"
  }, "MQ-5 + VOC + \u706B\u7130\u56FE\u50CF \xB7 \u4E09\u91CD\u786E\u8BA4"), /*#__PURE__*/React.createElement("div", {
    className: "saf-count"
  }, sealed ? "PC0" : count), /*#__PURE__*/React.createElement("div", {
    className: "saf-act"
  }, sealed ? "继电器吸合 · DN15 常闭阀已断气" : "即将自动关闭燃气电磁阀"), /*#__PURE__*/React.createElement("div", {
    className: "saf-sensors"
  }, /*#__PURE__*/React.createElement("div", {
    className: "saf-sensor"
  }, /*#__PURE__*/React.createElement("div", {
    className: "l"
  }, "MQ-5 \u71C3\u6C14"), /*#__PURE__*/React.createElement("div", {
    className: "v"
  }, "1820", /*#__PURE__*/React.createElement("span", null, "ppm"))), /*#__PURE__*/React.createElement("div", {
    className: "saf-sensor"
  }, /*#__PURE__*/React.createElement("div", {
    className: "l"
  }, "VOC"), /*#__PURE__*/React.createElement("div", {
    className: "v"
  }, "\u8D85\u9608\u503C")), /*#__PURE__*/React.createElement("div", {
    className: "saf-sensor"
  }, /*#__PURE__*/React.createElement("div", {
    className: "l"
  }, "\u54CD\u5E94\u5EF6\u8FDF"), /*#__PURE__*/React.createElement("div", {
    className: "v"
  }, "87", /*#__PURE__*/React.createElement("span", null, "ms"))), /*#__PURE__*/React.createElement("div", {
    className: "saf-sensor"
  }, /*#__PURE__*/React.createElement("div", {
    className: "l"
  }, "\u6267\u884C"), /*#__PURE__*/React.createElement("div", {
    className: "v"
  }, "PC0 \u5173\u9600"))), /*#__PURE__*/React.createElement("button", {
    className: "saf-cancel",
    onClick: onCancel
  }, "\u6211\u5DF2\u5904\u7406 \xB7 \u89E3\u9664\u8B66\u62A5"), /*#__PURE__*/React.createElement("div", {
    className: "saf-note"
  }, "\u5B89\u5168\u95ED\u73AF\u5B8C\u5168\u5728 STM32 \u672C\u5730\u8FD0\u884C \xB7 \u65AD\u7F51\u4ECD\u53EF\u9760\u5173\u9600"));
}
function CamFeed() {
  return /*#__PURE__*/React.createElement("div", {
    className: "cam-feed",
    "aria-hidden": "true"
  }, /*#__PURE__*/React.createElement("div", {
    className: "wall-grid"
  }), /*#__PURE__*/React.createElement("div", {
    className: "counter"
  }), /*#__PURE__*/React.createElement("div", {
    className: "cooktop"
  }, /*#__PURE__*/React.createElement("span", {
    className: "burner b1"
  }), /*#__PURE__*/React.createElement("span", {
    className: "burner b2"
  }), /*#__PURE__*/React.createElement("span", {
    className: "burner b3"
  })), /*#__PURE__*/React.createElement("div", {
    className: "pan"
  }, /*#__PURE__*/React.createElement("span", {
    className: "pan-core"
  }), /*#__PURE__*/React.createElement("span", {
    className: "steam s1"
  }), /*#__PURE__*/React.createElement("span", {
    className: "steam s2"
  }), /*#__PURE__*/React.createElement("span", {
    className: "steam s3"
  })), /*#__PURE__*/React.createElement("div", {
    className: "ingredient plate-a"
  }), /*#__PURE__*/React.createElement("div", {
    className: "ingredient plate-b"
  }), /*#__PURE__*/React.createElement("div", {
    className: "knife-line"
  }));
}
Object.assign(window, {
  RECIPE: RECIPE,
  fmtSecs: fmtSecs,
  fmtTime: fmtTime,
  StatusBar: StatusBar,
  StandbyHome: StandbyHome,
  TempGauge: TempGauge,
  ARLayer: ARLayer,
  RecipePanel: RecipePanel,
  VoiceCaption: VoiceCaption,
  Scheduler: Scheduler,
  SafetyOverlay: SafetyOverlay,
  CamFeed: CamFeed,
  GestureToast: GestureToast
});

/* global React, ReactDOM, RECIPE, fmtSecs, StatusBar, StandbyHome, TempGauge, ARLayer, RecipePanel, VoiceCaption, Scheduler, SafetyOverlay, CamFeed, GestureToast */
var _React = React,
  useState = _React.useState,
  useEffect = _React.useEffect,
  useRef = _React.useRef,
  useCallback = _React.useCallback;
var TOTAL = RECIPE.steps.length;
var CONN_NOTICE = {
  online: "已切回 WiFi 在线模式,云端菜谱理解与语音对话可用",
  g4: "WiFi 心跳失败,已由 EC600S 4G 备用链路接管",
  offline: "已进入离线模式,AR 步骤、锅温读取与关阀闭环继续本地运行"
};
var URL_PARAMS = new URLSearchParams(window.location.search);
var INITIAL_VIEW = URL_PARAMS.get("view") || "standby";
var INITIAL_CONN = ["online", "g4", "offline"].includes(URL_PARAMS.get("conn")) ? URL_PARAMS.get("conn") : "online";
var INITIAL_HEAT = URL_PARAMS.get("heat") === "1";
function App() {
  var _useState = useState(new Date()),
    _useState2 = _slicedToArray(_useState, 2),
    clock = _useState2[0],
    setClock = _useState2[1];
  var _useState3 = useState(INITIAL_VIEW === "standby" ? "standby" : "cooking"),
    _useState4 = _slicedToArray(_useState3, 2),
    mode = _useState4[0],
    setMode = _useState4[1];
  var _useState5 = useState(INITIAL_VIEW === "schedule" ? "schedule" : "ar"),
    _useState6 = _slicedToArray(_useState5, 2),
    focus = _useState6[0],
    setFocus = _useState6[1];
  var _useState7 = useState(0),
    _useState8 = _slicedToArray(_useState7, 2),
    step = _useState8[0],
    setStep = _useState8[1];
  var _useState9 = useState(INITIAL_VIEW === "safety" ? 254 : INITIAL_HEAT ? 238 : 118),
    _useState0 = _slicedToArray(_useState9, 2),
    potTemp = _useState0[0],
    setPotTemp = _useState0[1];
  var _useState1 = useState(INITIAL_CONN),
    _useState10 = _slicedToArray(_useState1, 2),
    conn = _useState10[0],
    setConn = _useState10[1];
  var _useState11 = useState(false),
    _useState12 = _slicedToArray(_useState11, 2),
    voiceLive = _useState12[0],
    setVoiceLive = _useState12[1];
  var _useState13 = useState({
      html: "",
      show: false
    }),
    _useState14 = _slicedToArray(_useState13, 2),
    cap = _useState14[0],
    setCap = _useState14[1];
  var _useState15 = useState(INITIAL_VIEW === "safety"),
    _useState16 = _slicedToArray(_useState15, 2),
    alert = _useState16[0],
    setAlert = _useState16[1];
  var _useState17 = useState(5),
    _useState18 = _slicedToArray(_useState17, 2),
    count = _useState18[0],
    setCount = _useState18[1];
  var _useState19 = useState(96),
    _useState20 = _slicedToArray(_useState19, 2),
    fishSecs = _useState20[0],
    setFishSecs = _useState20[1];
  var _useState21 = useState(1),
    _useState22 = _slicedToArray(_useState21, 2),
    scale = _useState22[0],
    setScale = _useState22[1];
  var _useState23 = useState({
      show: false,
      dir: "next"
    }),
    _useState24 = _slicedToArray(_useState23, 2),
    gesture = _useState24[0],
    setGesture = _useState24[1];
  var _useState25 = useState(INITIAL_HEAT),
    _useState26 = _slicedToArray(_useState25, 2),
    heatBoost = _useState26[0],
    setHeatBoost = _useState26[1];
  var capTimer = useRef(null);
  var gestureTimer = useRef(null);
  var fishAnnounced = useRef(false);
  var tempAnnounced = useRef(false);
  useEffect(function () {
    var id = setInterval(function () {
      return setClock(new Date());
    }, 1000);
    return function () {
      return clearInterval(id);
    };
  }, []);
  useEffect(function () {
    var fit = function fit() {
      var usableW = Math.max(360, window.innerWidth - 72);
      var usableH = Math.max(260, window.innerHeight - 188);
      var s = Math.min(usableW / 1920, usableH / 1080);
      setScale(Math.max(0.22, s));
    };
    fit();
    window.addEventListener("resize", fit);
    return function () {
      return window.removeEventListener("resize", fit);
    };
  }, []);
  var showCap = useCallback(function (html) {
    setCap({
      html: html,
      show: true
    });
    setVoiceLive(true);
    clearTimeout(capTimer.current);
    capTimer.current = setTimeout(function () {
      setCap(function (c) {
        return _objectSpread(_objectSpread({}, c), {}, {
          show: false
        });
      });
      setVoiceLive(false);
    }, 4200);
  }, []);
  useEffect(function () {
    return function () {
      clearTimeout(capTimer.current);
      clearTimeout(gestureTimer.current);
    };
  }, []);
  useEffect(function () {
    if (mode !== "cooking" || alert) return;
    var target = heatBoost ? Math.max(236, RECIPE.steps[step].target) : RECIPE.steps[step].target;
    var id = setInterval(function () {
      setPotTemp(function (p) {
        var drift = target - p;
        var next = p + drift * 0.105 + (Math.random() - 0.5) * 2.4;
        return Math.round(next * 10) / 10;
      });
    }, 180);
    return function () {
      return clearInterval(id);
    };
  }, [mode, step, alert, heatBoost]);
  useEffect(function () {
    if (!heatBoost) return;
    var id = setTimeout(function () {
      return setHeatBoost(false);
    }, 9000);
    return function () {
      return clearTimeout(id);
    };
  }, [heatBoost]);
  useEffect(function () {
    if (mode !== "cooking" || alert) return;
    var id = setInterval(function () {
      return setFishSecs(function (s) {
        return s > 0 ? s - 1 : 0;
      });
    }, 1000);
    return function () {
      return clearInterval(id);
    };
  }, [mode, alert]);
  useEffect(function () {
    if (fishSecs === 30 && !fishAnnounced.current) {
      fishAnnounced.current = true;
      showCap("清蒸鱼还有 <b>30 秒</b>,建议准备关火取出");
    }
  }, [fishSecs, showCap]);
  useEffect(function () {
    if (mode === "cooking" && !alert) {
      showCap(RECIPE.steps[step].voice);
    }
  }, [step, mode, alert, showCap]);
  useEffect(function () {
    if (mode !== "cooking" || alert) return;
    if ((potTemp > 220 || heatBoost) && !tempAnnounced.current) {
      tempAnnounced.current = true;
      showCap("锅温超过 <b>220℃</b>,系统建议调小火或立即下锅");
    }
    if (potTemp < 205 && !heatBoost && tempAnnounced.current) {
      tempAnnounced.current = false;
    }
  }, [potTemp, heatBoost, mode, alert, showCap]);
  useEffect(function () {
    if (!alert || count <= 0) return;
    var id = setTimeout(function () {
      return setCount(function (c) {
        return c - 1;
      });
    }, 1000);
    return function () {
      return clearTimeout(id);
    };
  }, [alert, count]);
  var pulseGesture = useCallback(function (dir) {
    setGesture({
      show: true,
      dir: dir
    });
    clearTimeout(gestureTimer.current);
    gestureTimer.current = setTimeout(function () {
      return setGesture(function (g) {
        return _objectSpread(_objectSpread({}, g), {}, {
          show: false
        });
      });
    }, 980);
  }, []);
  var next = useCallback(function () {
    setStep(function (s) {
      return Math.min(TOTAL - 1, s + 1);
    });
    pulseGesture("next");
  }, [pulseGesture]);
  var prev = useCallback(function () {
    setStep(function (s) {
      return Math.max(0, s - 1);
    });
    pulseGesture("prev");
  }, [pulseGesture]);
  useEffect(function () {
    var h = function h(e) {
      if (mode !== "cooking") return;
      if (e.key === "ArrowRight") next();
      if (e.key === "ArrowLeft") prev();
    };
    window.addEventListener("keydown", h);
    return function () {
      return window.removeEventListener("keydown", h);
    };
  }, [mode, next, prev]);
  var startCooking = useCallback(function () {
    setMode("cooking");
    setFocus("ar");
    setStep(0);
    setPotTemp(132);
    setFishSecs(96);
    setHeatBoost(false);
    fishAnnounced.current = false;
  }, []);
  var setConnection = useCallback(function (nextConn) {
    setConn(nextConn);
    showCap(CONN_NOTICE[nextConn]);
  }, [showCap]);
  var cycleConn = useCallback(function () {
    var order = ["online", "g4", "offline"];
    var nextConn = order[(order.indexOf(conn) + 1) % order.length];
    setConnection(nextConn);
  }, [conn, setConnection]);
  var wake = useCallback(function () {
    if (conn === "offline") {
      showCap("离线唤醒已响应:支持下一步、上一步、锅温与紧急关阀");
      return;
    }
    showCap("我在,你可以说 <b>下一步</b>、<b>火大了吗</b> 或 <b>开始调度</b>");
  }, [conn, showCap]);
  var forceHeat = useCallback(function () {
    setMode("cooking");
    setFocus("ar");
    setHeatBoost(true);
    setPotTemp(238);
    showCap("红外阵列检测到锅心温度快速上升,已进入锅温预警");
  }, [showCap]);
  var triggerAlert = useCallback(function () {
    setMode("cooking");
    setFocus("ar");
    setHeatBoost(false);
    setPotTemp(254);
    setCount(5);
    setAlert(true);
  }, []);
  var clearAlert = useCallback(function () {
    setAlert(false);
    setCount(5);
    setHeatBoost(false);
    setPotTemp(188);
    showCap("警报已解除,燃气阀保持关闭前请完成现场检查");
  }, [showCap]);
  var showStandby = function showStandby() {
    setMode("standby");
    setFocus("ar");
    setAlert(false);
  };
  var showAr = function showAr() {
    setMode("cooking");
    setFocus("ar");
    setAlert(false);
  };
  var showSchedule = function showSchedule() {
    setMode("cooking");
    setFocus("schedule");
    setAlert(false);
  };
  var st = RECIPE.steps[step];
  var isCooking = mode === "cooking";
  return /*#__PURE__*/React.createElement("div", {
    className: "proto-root"
  }, /*#__PURE__*/React.createElement("div", {
    className: "mirror-outer",
    "aria-label": "CookMirror 16:9 mirror prototype"
  }, /*#__PURE__*/React.createElement("div", {
    style: {
      width: 1920 * scale + "px",
      height: 1080 * scale + "px"
    }
  }, /*#__PURE__*/React.createElement("div", {
    className: "mirror-stage conn-" + conn + (alert ? " alerting" : ""),
    style: {
      transform: "scale(".concat(scale, ")")
    }
  }, /*#__PURE__*/React.createElement("div", {
    className: "mirror-scene"
  }, /*#__PURE__*/React.createElement(CamFeed, null), /*#__PURE__*/React.createElement("div", {
    className: "scene-veil"
  }), /*#__PURE__*/React.createElement("div", {
    className: "scene-sheen"
  }), /*#__PURE__*/React.createElement("div", {
    className: "scanlines"
  })), /*#__PURE__*/React.createElement(StatusBar, {
    clock: clock,
    conn: conn,
    onCycleConn: cycleConn,
    voiceLive: voiceLive,
    onVoice: wake
  }), mode === "standby" && /*#__PURE__*/React.createElement(StandbyHome, {
    clock: clock,
    conn: conn,
    onStart: startCooking
  }), isCooking && /*#__PURE__*/React.createElement("div", {
    className: "hud focus-" + focus
  }, /*#__PURE__*/React.createElement(ARLayer, {
    step: st,
    muted: focus === "schedule"
  }), /*#__PURE__*/React.createElement(TempGauge, {
    temp: potTemp,
    target: st.target,
    pos: st.tempPos
  }), /*#__PURE__*/React.createElement(RecipePanel, {
    step: st,
    idx: step,
    total: TOTAL,
    timer: fmtSecs(Math.max(0, 180 - step * 20)),
    conn: conn,
    thermalWarn: potTemp > 220 || heatBoost
  }), /*#__PURE__*/React.createElement(VoiceCaption, {
    html: cap.html,
    show: cap.show
  }), /*#__PURE__*/React.createElement(GestureToast, {
    gesture: gesture
  }), /*#__PURE__*/React.createElement("div", {
    className: "gesture-hint"
  }, /*#__PURE__*/React.createElement("span", {
    className: "gesture-mark"
  }), /*#__PURE__*/React.createElement("span", null, "LD2410B \u624B\u52BF\u5C31\u7EEA"), /*#__PURE__*/React.createElement("b", null, step + 1, "/", TOTAL)), /*#__PURE__*/React.createElement(Scheduler, {
    activeStep: step,
    total: TOTAL,
    fishSecs: fishSecs,
    focus: focus === "schedule",
    onFocus: function onFocus() {
      return setFocus("schedule");
    }
  })), alert && /*#__PURE__*/React.createElement(SafetyOverlay, {
    count: count,
    onCancel: clearAlert
  })))), /*#__PURE__*/React.createElement("div", {
    className: "demo-dock",
    "aria-label": "\u6F14\u793A\u63A7\u5236\u53F0"
  }, /*#__PURE__*/React.createElement("div", {
    className: "grp"
  }, /*#__PURE__*/React.createElement("span", {
    className: "gl"
  }, "\u6D41\u7A0B"), /*#__PURE__*/React.createElement("button", {
    className: "dbtn" + (mode === "standby" ? " on" : ""),
    onClick: showStandby
  }, "\u5F85\u673A\u955C\u9762"), /*#__PURE__*/React.createElement("button", {
    className: "dbtn" + (isCooking && focus === "ar" && !alert ? " on" : ""),
    onClick: showAr
  }, "AR \u5F15\u5BFC HUD"), /*#__PURE__*/React.createElement("button", {
    className: "dbtn" + (isCooking && focus === "schedule" && !alert ? " on" : ""),
    onClick: showSchedule
  }, "\u591A\u83DC\u8C03\u5EA6"), /*#__PURE__*/React.createElement("button", {
    className: "dbtn" + (alert ? " on danger" : ""),
    onClick: triggerAlert
  }, "\u5B89\u5168\u5173\u9600")), /*#__PURE__*/React.createElement("div", {
    className: "grp"
  }, /*#__PURE__*/React.createElement("span", {
    className: "gl"
  }, "\u9694\u7A7A\u624B\u52BF"), /*#__PURE__*/React.createElement("button", {
    className: "dbtn",
    onClick: prev
  }, "\u4E0A\u4E00\u6B65"), /*#__PURE__*/React.createElement("button", {
    className: "dbtn",
    onClick: next
  }, "\u4E0B\u4E00\u6B65")), /*#__PURE__*/React.createElement("div", {
    className: "grp"
  }, /*#__PURE__*/React.createElement("span", {
    className: "gl"
  }, "\u94FE\u8DEF"), /*#__PURE__*/React.createElement("button", {
    className: "dbtn" + (conn === "online" ? " on" : ""),
    onClick: function onClick() {
      return setConnection("online");
    }
  }, "\u5728\u7EBF"), /*#__PURE__*/React.createElement("button", {
    className: "dbtn" + (conn === "g4" ? " on" : ""),
    onClick: function onClick() {
      return setConnection("g4");
    }
  }, "4G"), /*#__PURE__*/React.createElement("button", {
    className: "dbtn" + (conn === "offline" ? " on" : ""),
    onClick: function onClick() {
      return setConnection("offline");
    }
  }, "\u79BB\u7EBF")), /*#__PURE__*/React.createElement("div", {
    className: "grp"
  }, /*#__PURE__*/React.createElement("span", {
    className: "gl"
  }, "\u4F20\u611F\u5668"), /*#__PURE__*/React.createElement("button", {
    className: "dbtn",
    onClick: wake
  }, "\u8BED\u97F3\u5524\u9192"), /*#__PURE__*/React.createElement("button", {
    className: "dbtn warn",
    onClick: forceHeat
  }, "\u9505\u6E29\u9884\u8B66")), /*#__PURE__*/React.createElement("span", {
    className: "demo-hint"
  }, "CookMirror \u53A8\u9B54\u955C \xB7 16:9 \u540E\u7F6E LCD \u955C\u9762\u4EA4\u4E92\u539F\u578B")));
}
ReactDOM.createRoot(document.getElementById("root")).render(/*#__PURE__*/React.createElement(App, null));
