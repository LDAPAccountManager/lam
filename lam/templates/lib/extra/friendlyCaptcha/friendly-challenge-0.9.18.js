"use strict";

function _typeof(o) { "@babel/helpers - typeof"; return _typeof = "function" == typeof Symbol && "symbol" == typeof Symbol.iterator ? function (o) { return typeof o; } : function (o) { return o && "function" == typeof Symbol && o.constructor === Symbol && o !== Symbol.prototype ? "symbol" : typeof o; }, _typeof(o); }
function _classCallCheck(a, n) { if (!(a instanceof n)) throw new TypeError("Cannot call a class as a function"); }
function _defineProperties(e, r) { for (var t = 0; t < r.length; t++) { var o = r[t]; o.enumerable = o.enumerable || !1, o.configurable = !0, "value" in o && (o.writable = !0), Object.defineProperty(e, _toPropertyKey(o.key), o); } }
function _createClass(e, r, t) { return r && _defineProperties(e.prototype, r), t && _defineProperties(e, t), Object.defineProperty(e, "prototype", { writable: !1 }), e; }
function _toPropertyKey(t) { var i = _toPrimitive(t, "string"); return "symbol" == _typeof(i) ? i : i + ""; }
function _toPrimitive(t, r) { if ("object" != _typeof(t) || !t) return t; var e = t[Symbol.toPrimitive]; if (void 0 !== e) { var i = e.call(t, r || "default"); if ("object" != _typeof(i)) return i; throw new TypeError("@@toPrimitive must return a primitive value."); } return ("string" === r ? String : Number)(t); }
function _slicedToArray(r, e) { return _arrayWithHoles(r) || _iterableToArrayLimit(r, e) || _unsupportedIterableToArray(r, e) || _nonIterableRest(); }
function _nonIterableRest() { throw new TypeError("Invalid attempt to destructure non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); }
function _unsupportedIterableToArray(r, a) { if (r) { if ("string" == typeof r) return _arrayLikeToArray(r, a); var t = {}.toString.call(r).slice(8, -1); return "Object" === t && r.constructor && (t = r.constructor.name), "Map" === t || "Set" === t ? Array.from(r) : "Arguments" === t || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(t) ? _arrayLikeToArray(r, a) : void 0; } }
function _arrayLikeToArray(r, a) { (null == a || a > r.length) && (a = r.length); for (var e = 0, n = Array(a); e < a; e++) n[e] = r[e]; return n; }
function _iterableToArrayLimit(r, l) { var t = null == r ? null : "undefined" != typeof Symbol && r[Symbol.iterator] || r["@@iterator"]; if (null != t) { var e, n, i, u, a = [], f = !0, o = !1; try { if (i = (t = t.call(r)).next, 0 === l) { if (Object(t) !== t) return; f = !1; } else for (; !(f = (e = i.call(t)).done) && (a.push(e.value), a.length !== l); f = !0); } catch (r) { o = !0, n = r; } finally { try { if (!f && null != t.return && (u = t.return(), Object(u) !== u)) return; } finally { if (o) throw n; } } return a; } }
function _arrayWithHoles(r) { if (Array.isArray(r)) return r; }
(function () {
  'use strict';

  var css = '.frc-captcha *{margin:0;padding:0;border:0;text-align:initial;border-radius:0;filter:none!important;transition:none!important;font-weight:400;font-size:14px;line-height:1.2;text-decoration:none;background-color:initial;color:#222}.frc-captcha{position:relative;min-width:250px;max-width:312px;border:1px solid #f4f4f4;padding-bottom:12px;background-color:#fff}.frc-captcha b{font-weight:700}.frc-container{display:flex;align-items:center;min-height:52px}.frc-icon{fill:#222;stroke:#222;flex-shrink:0;margin:8px 8px 0}.frc-icon.frc-warning{fill:#c00}.frc-success .frc-icon{animation:1s ease-in both frc-fade-in}.frc-content{white-space:nowrap;display:flex;flex-direction:column;margin:4px 6px 0 0;overflow-x:auto;flex-grow:1}.frc-banner{position:absolute;bottom:0;right:6px;line-height:1}.frc-banner *{font-size:10px;opacity:.8;text-decoration:none}.frc-progress{-webkit-appearance:none;-moz-appearance:none;appearance:none;margin:3px 0;height:4px;border:none;background-color:#eee;color:#222;width:100%;transition:.5s linear}.frc-progress::-webkit-progress-bar{background:#eee}.frc-progress::-webkit-progress-value{background:#222}.frc-progress::-moz-progress-bar{background:#222}.frc-button{cursor:pointer;padding:2px 6px;background-color:#f1f1f1;border:1px solid transparent;text-align:center;font-weight:600;text-transform:none}.frc-button:focus{border:1px solid #333}.frc-button:hover{background-color:#ddd}.frc-captcha-solution{display:none}.frc-err-url{text-decoration:underline;font-size:.9em}.frc-rtl{direction:rtl}.frc-rtl .frc-content{margin:4px 0 0 6px}.frc-banner.frc-rtl{left:6px;right:auto}.dark.frc-captcha{color:#fff;background-color:#222;border-color:#333}.dark.frc-captcha *{color:#fff}.dark.frc-captcha button{background-color:#444}.dark .frc-icon{fill:#fff;stroke:#fff}.dark .frc-progress{background-color:#444}.dark .frc-progress::-webkit-progress-bar{background:#444}.dark .frc-progress::-webkit-progress-value{background:#ddd}.dark .frc-progress::-moz-progress-bar{background:#ddd}@keyframes frc-fade-in{from{opacity:0}to{opacity:1}}';

  // This is not an enum to save some bytes in the output bundle.
  var SOLVER_TYPE_JS = 1;
  var CHALLENGE_SIZE_BYTES = 128;

  // @ts-ignore
  var loaderSVG = "<circle cx=\"12\" cy=\"12\" r=\"8\" stroke-width=\"3\" stroke-dasharray=\"15 10\" fill=\"none\" stroke-linecap=\"round\" transform=\"rotate(0 12 12)\"><animateTransform attributeName=\"transform\" type=\"rotate\" repeatCount=\"indefinite\" dur=\"0.9s\" values=\"0 12 12;360 12 12\"/></circle>";
  var errorSVG = "<path d=\"M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-2h2v2zm0-4h-2V7h2v6z\"/>";
  /**
   * Base template used for all widget states
   * The reason we use raw string interpolation here is so we don't have to ship something like lit-html.
   */
  function getTemplate(fieldName, rtl, svgContent, svgAriaHidden, textContent, solutionString, buttonText) {
    var progress = arguments.length > 7 && arguments[7] !== undefined ? arguments[7] : false;
    var debugData = arguments.length > 8 ? arguments[8] : undefined;
    var additionalContainerClasses = arguments.length > 9 ? arguments[9] : undefined;
    return "<div class=\"frc-container".concat(additionalContainerClasses ? " " + additionalContainerClasses : "").concat(rtl ? " frc-rtl" : "", "\">\n<svg class=\"frc-icon\"").concat(svgAriaHidden ? ' aria-hidden="true"' : "", " role=\"img\" xmlns=\"http://www.w3.org/2000/svg\" height=\"32\" width=\"32\" viewBox=\"0 0 24 24\">").concat(svgContent, "</svg>\n<div class=\"frc-content\">\n    <span class=\"frc-text\" ").concat(debugData ? "data-debug=\"".concat(debugData, "\"") : "", ">").concat(textContent, "</span>\n    ").concat(buttonText ? "<button type=\"button\" class=\"frc-button\">".concat(buttonText, "</button>") : "", "\n    ").concat(progress ? "<progress class=\"frc-progress\" value=\"0\">0%</progress>" : "", "\n</div>\n</div><span class=\"frc-banner").concat(rtl ? " frc-rtl" : "", "\"><a lang=\"en\" href=\"https://friendlycaptcha.com/\" rel=\"noopener\" target=\"_blank\"><b>Friendly</b>Captcha \u21D7</a></span>\n").concat(fieldName === "-" ? "" : "<input name=\"".concat(fieldName, "\" class=\"frc-captcha-solution\" type=\"hidden\" value=\"").concat(solutionString, "\">"));
  }
  /**
   * Used when the widget is ready to start solving.
   */
  function getReadyHTML(fieldName, l) {
    return getTemplate(fieldName, l.rtl, "<path d=\"M17,11c0.34,0,0.67,0.04,1,0.09V6.27L10.5,3L3,6.27v4.91c0,4.54,3.2,8.79,7.5,9.82c0.55-0.13,1.08-0.32,1.6-0.55 C11.41,19.47,11,18.28,11,17C11,13.69,13.69,11,17,11z\"/><path d=\"M17,13c-2.21,0-4,1.79-4,4c0,2.21,1.79,4,4,4s4-1.79,4-4C21,14.79,19.21,13,17,13z M17,14.38\"/>", true, l.text_ready, ".UNSTARTED", l.button_start, false);
  }
  /**
   * Used when the widget is retrieving a puzzle
   */
  function getFetchingHTML(fieldName, l) {
    return getTemplate(fieldName, l.rtl, loaderSVG, true, l.text_fetching, ".FETCHING", undefined, true);
  }
  /**
   * Used when the solver is running, displays a progress bar.
   */
  function getRunningHTML(fieldName, l) {
    return getTemplate(fieldName, l.rtl, loaderSVG, true, l.text_solving, ".UNFINISHED", undefined, true);
  }
  function getDoneHTML(fieldName, l, solution, data) {
    var timeData = "".concat(data.t.toFixed(0), "s (").concat((data.h / data.t * 0.001).toFixed(0), "K/s)").concat(data.solver === SOLVER_TYPE_JS ? " JS Fallback" : "");
    return getTemplate(fieldName, l.rtl, "<title>".concat(l.text_completed_sr, "</title><path d=\"M12 1L3 5v6c0 5.55 3.84 10.74 9 12 5.16-1.26 9-6.45 9-12V5l-9-4zm-2 16l-4-4 1.41-1.41L10 14.17l6.59-6.59L18 9l-8 8z\"></path>"), false, l.text_completed, solution, undefined, false, timeData, "frc-success");
  }
  function getExpiredHTML(fieldName, l) {
    return getTemplate(fieldName, l.rtl, errorSVG, true, l.text_expired, ".EXPIRED", l.button_restart);
  }
  function getErrorHTML(fieldName, l, errorDescription) {
    var recoverable = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : true;
    var headless = arguments.length > 4 && arguments[4] !== undefined ? arguments[4] : false;
    return getTemplate(fieldName, l.rtl, errorSVG, true, "<b>".concat(l.text_error, "</b><br>").concat(errorDescription), headless ? ".HEADLESS_ERROR" : ".ERROR", recoverable ? l.button_retry : undefined);
  }
  function findCaptchaElements() {
    var elements = document.querySelectorAll(".frc-captcha");
    if (elements.length === 0) {
      console.warn("FriendlyCaptcha: No div was found with .frc-captcha class");
    }
    return elements;
  }
  /**
   * Injects the style if no #frc-style element is already present
   * (to support custom stylesheets)
   */
  function injectStyle() {
    var styleNonce = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : null;
    if (!document.querySelector("#frc-style")) {
      var styleSheet = document.createElement("style");
      styleSheet.id = "frc-style";
      styleSheet.innerHTML = css;
      if (styleNonce) {
        styleSheet.setAttribute('nonce', styleNonce);
      }
      document.head.appendChild(styleSheet);
    }
  }
  /**
   * @param element parent element of friendlycaptcha
   * @param progress value between 0 and 1
   */
  function updateProgressBar(element, data) {
    var p = element.querySelector(".frc-progress");
    var perc = (data.i + 1) / data.n;
    if (p) {
      p.value = perc;
      p.innerText = (perc * 100).toFixed(1) + "%";
      p.title = data.i + 1 + "/" + data.n + " (" + (data.h / data.t * 0.001).toFixed(0) + "K/s)";
    }
  }
  /**
   * Traverses parent nodes until a <form> is found, returns null if not found.
   */
  function findParentFormElement(element) {
    while (element.tagName !== "FORM") {
      element = element.parentElement;
      if (!element) {
        return null;
      }
    }
    return element;
  }
  /**
   * Add listener to specified element that will only fire once on focus.
   */
  function executeOnceOnFocusInEvent(element, listener) {
    element.addEventListener("focusin", listener, {
      once: true,
      passive: true
    });
  }

  // Adapted from the base64-arraybuffer package implementation
  // (https://github.com/niklasvh/base64-arraybuffer, MIT licensed)
  var CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  var EQ_CHAR = "=".charCodeAt(0);
  // Use a lookup table to find the index.
  var lookup = new Uint8Array(256);
  for (var i = 0; i < CHARS.length; i++) {
    lookup[CHARS.charCodeAt(i)] = i;
  }
  function encode(bytes) {
    var len = bytes.length;
    var base64 = "";
    for (var _i = 0; _i < len; _i += 3) {
      var b0 = bytes[_i + 0];
      var b1 = bytes[_i + 1];
      var b2 = bytes[_i + 2];
      // This temporary variable stops the NextJS 13 compiler from breaking this code in optimization.
      // See issue https://github.com/FriendlyCaptcha/friendly-challenge/issues/165
      var t = "";
      t += CHARS.charAt(b0 >>> 2);
      t += CHARS.charAt((b0 & 3) << 4 | b1 >>> 4);
      t += CHARS.charAt((b1 & 15) << 2 | b2 >>> 6);
      t += CHARS.charAt(b2 & 63);
      base64 += t;
    }
    if (len % 3 === 2) {
      base64 = base64.substring(0, base64.length - 1) + "=";
    } else if (len % 3 === 1) {
      base64 = base64.substring(0, base64.length - 2) + "==";
    }
    return base64;
  }
  function decode(base64) {
    var len = base64.length;
    var bufferLength = len * 3 >>> 2; // * 0.75
    if (base64.charCodeAt(len - 1) === EQ_CHAR) bufferLength--;
    if (base64.charCodeAt(len - 2) === EQ_CHAR) bufferLength--;
    var bytes = new Uint8Array(bufferLength);
    for (var _i2 = 0, p = 0; _i2 < len; _i2 += 4) {
      var encoded1 = lookup[base64.charCodeAt(_i2 + 0)];
      var encoded2 = lookup[base64.charCodeAt(_i2 + 1)];
      var encoded3 = lookup[base64.charCodeAt(_i2 + 2)];
      var encoded4 = lookup[base64.charCodeAt(_i2 + 3)];
      bytes[p++] = encoded1 << 2 | encoded2 >> 4;
      bytes[p++] = (encoded2 & 15) << 4 | encoded3 >> 2;
      bytes[p++] = (encoded3 & 3) << 6 | encoded4 & 63;
    }
    return bytes;
  }

  // Defensive init to make it easier to integrate with Gatsby, NextJS, and friends.
  var nav;
  var ua;
  if (typeof navigator !== "undefined" && typeof navigator.userAgent === "string") {
    nav = navigator;
    ua = nav.userAgent.toLowerCase();
  }
  /**
   * Headless browser detection on the clientside is imperfect. One can modify any clientside code to disable or change this check,
   * and one can spoof whatever is checked here. However, that doesn't make it worthless: it's yet another hurdle for spammers and
   * it stops unsophisticated scripters from making any request whatsoever.
   */
  function isHeadless() {
    return (
      //tell-tale bot signs
      ua.indexOf("headless") !== -1 || nav.appVersion.indexOf("Headless") !== -1 || ua.indexOf("bot") !== -1 ||
      // http://www.useragentstring.com/pages/useragentstring.php?typ=Browser
      ua.indexOf("crawl") !== -1 ||
      // Only IE5 has two distributions that has this on windows NT.. so yeah.
      nav.webdriver === true || !nav.language || nav.languages !== undefined && !nav.languages.length // IE 11 does not support NavigatorLanguage.languages https://developer.mozilla.org/en-US/docs/Web/API/NavigatorLanguage/languages
    );
  }

  /**
   * Maps a value between 0 and 255 to a difficulty threshold (as uint32)
   * Difficulty 0 maps to 99.99% probability of being right on the first attempt
   * Anything above 250 needs 2^32 tries on average to solve.
   * 150 to 180 seems reasonable
   */
  function difficultyToThreshold(value) {
    if (value > 255) {
      value = 255;
    } else if (value < 0) {
      value = 0;
    }
    return Math.pow(2, (255.999 - value) / 8.0) >>> 0;
  }
  var PUZZLE_EXPIRY_OFFSET = 13;
  var NUMBER_OF_PUZZLES_OFFSET = 14;
  var PUZZLE_DIFFICULTY_OFFSET = 15;
  function getPuzzleSolverInputs(puzzleBuffer, numPuzzles) {
    var startingPoints = [];
    for (var _i3 = 0; _i3 < numPuzzles; _i3++) {
      var input = new Uint8Array(CHALLENGE_SIZE_BYTES);
      input.set(puzzleBuffer);
      input[120] = _i3;
      startingPoints.push(input);
    }
    return startingPoints;
  }
  function decodeBase64Puzzle(base64Puzzle) {
    var parts = base64Puzzle.split(".");
    var puzzle = parts[1];
    var arr = decode(puzzle);
    return {
      signature: parts[0],
      base64: puzzle,
      buffer: arr,
      n: arr[NUMBER_OF_PUZZLES_OFFSET],
      threshold: difficultyToThreshold(arr[PUZZLE_DIFFICULTY_OFFSET]),
      expiry: arr[PUZZLE_EXPIRY_OFFSET] * 300000
    };
  }
  function getPuzzle(urlsSeparatedByComma, siteKey, lang) {
    return new Promise(function ($return, $error) {
      var urls;
      urls = urlsSeparatedByComma.split(",");
      var $Loop_4_trampoline, $Loop_4_local;
      function $Loop_4_step() {
        var _$Loop_4_local = $Loop_4_local(),
          _$Loop_4_local2 = _slicedToArray(_$Loop_4_local, 1),
          i = _$Loop_4_local2[0];
        i++;
        return $Loop_4.bind(this, i);
      }
      function $Loop_4(i) {
        $Loop_4_local = function $Loop_4_local() {
          return [i];
        };
        if (i < urls.length) {
          var $Try_1_Post = function $Try_1_Post() {
            try {
              return $Loop_4_step;
            } catch ($boundEx) {
              return $error($boundEx);
            }
          };
          var $Try_1_Catch = function $Try_1_Catch(e) {
            try {
              {
                console.error("[FRC Fetch]:", e);
                var err;
                err = new Error("".concat(lang.text_fetch_error, " <a class=\"frc-err-url\" href=\"").concat(urls[i], "\">").concat(urls[i], "</a>"));
                err.rawError = e;
                throw err;
              }
            } catch ($boundEx) {
              return $error($boundEx);
            }
          };
          try {
            var response;
            return Promise.resolve(fetchAndRetryWithBackoff(urls[i] + "?sitekey=" + siteKey, {
              headers: [["x-frc-client", "js-0.9.18"]],
              mode: "cors"
            }, 2)).then(function ($await_7) {
              try {
                var $If_6 = function $If_6() {
                  return $Try_1_Post();
                };
                response = $await_7;
                if (response.ok) {
                  var json;
                  return Promise.resolve(response.json()).then(function ($await_8) {
                    try {
                      json = $await_8;
                      return $return(json.data.puzzle);
                    } catch ($boundEx) {
                      return $Try_1_Catch($boundEx);
                    }
                  }, $Try_1_Catch);
                } else {
                  var _json;
                  var $Try_2_Post = function () {
                    try {
                      if (_json && _json.errors && _json.errors[0] === "endpoint_not_enabled") {
                        throw Error("Endpoint not allowed (".concat(response.status, ")"));
                      }
                      if (i === urls.length - 1) {
                        throw Error("Response status ".concat(response.status, " ").concat(response.statusText, " ").concat(_json ? _json.errors : ""));
                      }
                      return $If_6.call(this);
                    } catch ($boundEx) {
                      return $Try_1_Catch($boundEx);
                    }
                  }.bind(this);
                  var $Try_2_Catch = function $Try_2_Catch(e) {
                    try {
                      return $Try_2_Post();
                    } catch ($boundEx) {
                      return $Try_1_Catch($boundEx);
                    }
                  } /* Do nothing, the error is not valid JSON */;
                  try {
                    return Promise.resolve(response.json()).then(function ($await_9) {
                      try {
                        _json = $await_9;
                        return $Try_2_Post();
                      } catch ($boundEx) {
                        return $Try_2_Catch($boundEx);
                      }
                    }, $Try_2_Catch);
                  } catch (e) {
                    $Try_2_Catch(e);
                  }
                }
                return $If_6.call(this);
              } catch ($boundEx) {
                return $Try_1_Catch($boundEx);
              }
            }.bind(this), $Try_1_Catch);
          } catch (e) {
            $Try_1_Catch(e);
          }
        } else return [1];
      }
      return ($Loop_4_trampoline = function (q) {
        while (q) {
          if (q.then) return void q.then($Loop_4_trampoline, $error);
          try {
            if (q.pop) {
              if (q.length) return q.pop() ? $Loop_4_exit.call(this) : q;else q = $Loop_4_step;
            } else q = q.call(this);
          } catch (_exception) {
            return $error(_exception);
          }
        }
      }.bind(this))($Loop_4.bind(this, 0));
      function $Loop_4_exit() {
        // This code should never be reached.
        return $error(Error("Internal error"));
      }
    });
  }
  /**
   * Retries given request with exponential backoff (starting with 1000ms delay, multiplying by 4 every time)
   * @param url Request (can be string url) to fetch
   * @param opts Options for fetch
   * @param n Number of times to attempt before giving up.
   */
  function fetchAndRetryWithBackoff(url, opts, n) {
    return new Promise(function ($return, $error) {
      var time = 1000;
      return $return(fetch(url, opts).catch(function (error) {
        return new Promise(function ($return, $error) {
          if (n === 0) return $error(error);
          return Promise.resolve(new Promise(function (r) {
            return setTimeout(r, time);
          })).then(function ($await_10) {
            try {
              time *= 4;
              return $return(fetchAndRetryWithBackoff(url, opts, n - 1));
            } catch ($boundEx) {
              return $error($boundEx);
            }
          }, $error);
        });
      }));
    });
  }

  // English
  var LANG_EN = {
    text_init: "Initializing...",
    text_ready: "Anti-Robot Verification",
    button_start: "Click to start verification",
    text_fetching: "Fetching Challenge",
    text_solving: "Verifying you are human...",
    text_completed: "I am human",
    text_completed_sr: "Automatic spam check completed",
    text_expired: "Anti-Robot verification expired",
    button_restart: "Restart",
    text_error: "Verification failed",
    button_retry: "Retry",
    text_fetch_error: "Failed to connect to"
  };
  // French
  var LANG_FR = {
    text_init: "Chargement...",
    text_ready: "Vérification Anti-Robot",
    button_start: "Clique ici pour vérifier",
    text_fetching: "Chargement du défi",
    text_solving: "Nous vérifions que vous n'êtes pas un robot...",
    text_completed: "Je ne suis pas un robot",
    text_completed_sr: "Vérification automatique des spams terminée",
    text_expired: "Vérification anti-robot expirée",
    button_restart: "Redémarrer",
    text_error: "Échec de la vérification",
    button_retry: "Recommencer",
    text_fetch_error: "Problème de connexion avec"
  };
  // German
  var LANG_DE = {
    text_init: "Initialisierung...",
    text_ready: "Anti-Roboter-Verifizierung",
    button_start: "Hier klicken",
    text_fetching: "Herausforderung laden...",
    text_solving: "Verifizierung, dass Sie ein Mensch sind...",
    text_completed: "Ich bin ein Mensch",
    text_completed_sr: "Automatische Spamprüfung abgeschlossen",
    text_expired: "Verifizierung abgelaufen",
    button_restart: "Erneut starten",
    text_error: "Verifizierung fehlgeschlagen",
    button_retry: "Erneut versuchen",
    text_fetch_error: "Verbindungsproblem mit"
  };
  // Dutch
  var LANG_NL = {
    text_init: "Initializeren...",
    text_ready: "Anti-robotverificatie",
    button_start: "Klik om te starten",
    text_fetching: "Aan het laden...",
    text_solving: "Anti-robotverificatie bezig...",
    text_completed: "Ik ben een mens",
    text_completed_sr: "Automatische anti-spamcheck voltooid",
    text_expired: "Verificatie verlopen",
    button_restart: "Opnieuw starten",
    text_error: "Verificatie mislukt",
    button_retry: "Opnieuw proberen",
    text_fetch_error: "Verbinding mislukt met"
  };
  // Italian
  var LANG_IT = {
    text_init: "Inizializzazione...",
    text_ready: "Verifica Anti-Robot",
    button_start: "Clicca per iniziare",
    text_fetching: "Caricamento...",
    text_solving: "Verificando che sei umano...",
    text_completed: "Non sono un robot",
    text_completed_sr: "Controllo automatico dello spam completato",
    text_expired: "Verifica Anti-Robot scaduta",
    button_restart: "Ricomincia",
    text_error: "Verifica fallita",
    button_retry: "Riprova",
    text_fetch_error: "Problema di connessione con"
  };
  // Portuguese
  var LANG_PT = {
    text_init: "Inicializando...",
    text_ready: "Verificação Anti-Robô",
    button_start: "Clique para iniciar verificação",
    text_fetching: "Carregando...",
    text_solving: "Verificando se você é humano...",
    text_completed: "Eu sou humano",
    text_completed_sr: "Verificação automática de spam concluída",
    text_expired: "Verificação Anti-Robô expirada",
    button_restart: "Reiniciar",
    text_error: "Verificação falhou",
    button_retry: "Tentar novamente",
    text_fetch_error: "Falha de conexão com"
  };
  // Spanish
  var LANG_ES = {
    text_init: "Inicializando...",
    text_ready: "Verificación Anti-Robot",
    button_start: "Haga clic para iniciar la verificación",
    text_fetching: "Cargando desafío",
    text_solving: "Verificando que eres humano...",
    text_completed: "Soy humano",
    text_completed_sr: "Verificación automática de spam completada",
    text_expired: "Verificación Anti-Robot expirada",
    button_restart: "Reiniciar",
    text_error: "Ha fallado la verificación",
    button_retry: "Intentar de nuevo",
    text_fetch_error: "Error al conectarse a"
  };
  // Catalan
  var LANG_CA = {
    text_init: "Inicialitzant...",
    text_ready: "Verificació Anti-Robot",
    button_start: "Fes clic per començar la verificació",
    text_fetching: "Carregant repte",
    text_solving: "Verificant que ets humà...",
    text_completed: "Soc humà",
    text_completed_sr: "Verificació automàtica de correu brossa completada",
    text_expired: "La verificació Anti-Robot ha expirat",
    button_restart: "Reiniciar",
    text_error: "Ha fallat la verificació",
    button_retry: "Tornar a provar",
    text_fetch_error: "Error connectant a"
  };
  // Japanese
  var LANG_JA = {
    text_init: "開始しています...",
    text_ready: "アンチロボット認証",
    button_start: "クリックして認証を開始",
    text_fetching: "ロードしています",
    text_solving: "認証中...",
    text_completed: "私はロボットではありません",
    text_completed_sr: "自動スパムチェックが完了しました",
    text_expired: "認証の期限が切れています",
    button_restart: "再度認証を行う",
    text_error: "認証にエラーが発生しました",
    button_retry: "再度認証を行う",
    text_fetch_error: "接続ができませんでした"
  };
  // Danish
  var LANG_DA = {
    text_init: "Aktiverer...",
    text_ready: "Jeg er ikke en robot",
    button_start: "Klik for at starte verifikationen",
    text_fetching: "Henter data",
    text_solving: "Kontrollerer at du er et menneske...",
    text_completed: "Jeg er et menneske.",
    text_completed_sr: "Automatisk spamkontrol gennemført",
    text_expired: "Verifikationen kunne ikke fuldføres",
    button_restart: "Genstart",
    text_error: "Bekræftelse mislykkedes",
    button_retry: "Prøv igen",
    text_fetch_error: "Forbindelsen mislykkedes"
  };
  // Russian
  var LANG_RU = {
    text_init: "Инициализация...",
    text_ready: "АнтиРобот проверка",
    button_start: "Нажмите, чтобы начать проверку",
    text_fetching: "Получаю задачу",
    text_solving: "Проверяю, что вы человек...",
    text_completed: "Я человек",
    text_completed_sr: "Aвтоматическая проверка на спам завершена",
    text_expired: "Срок АнтиРоботной проверки истёк",
    button_restart: "Начать заново",
    text_error: "Ошибка проверки",
    button_retry: "Повторить ещё раз",
    text_fetch_error: "Ошибка подключения"
  };
  // Swedish
  var LANG_SV = {
    text_init: "Aktiverar...",
    text_ready: "Jag är inte en robot",
    button_start: "Klicka för att verifiera",
    text_fetching: "Hämtar data",
    text_solving: "Kontrollerar att du är människa...",
    text_completed: "Jag är en människa",
    text_completed_sr: "Automatisk spamkontroll slutförd",
    text_expired: "Anti-robot-verifieringen har löpt ut",
    button_restart: "Börja om",
    text_error: "Verifiering kunde inte slutföras",
    button_retry: "Omstart",
    text_fetch_error: "Verifiering misslyckades"
  };
  // Turkish
  var LANG_TR = {
    text_init: "Başlatılıyor...",
    text_ready: "Anti-Robot Doğrulaması",
    button_start: "Doğrulamayı başlatmak için tıklayın",
    text_fetching: "Yükleniyor",
    text_solving: "Robot olmadığınız doğrulanıyor...",
    text_completed: "Ben bir insanım",
    text_completed_sr: "Otomatik spam kontrolü tamamlandı",
    text_expired: "Anti-Robot doğrulamasının süresi doldu",
    button_restart: "Yeniden başlat",
    text_error: "Doğrulama başarısız oldu",
    button_retry: "Tekrar dene",
    text_fetch_error: "Bağlantı başarısız oldu"
  };
  // Greek
  var LANG_EL = {
    text_init: "Προετοιμασία...",
    text_ready: "Anti-Robot Επαλήθευση",
    button_start: " Κάντε κλικ για να ξεκινήσει η επαλήθευση",
    text_fetching: " Λήψη πρόκλησης",
    text_solving: " Επιβεβαίωση ανθρώπου...",
    text_completed: "Είμαι άνθρωπος",
    text_completed_sr: " Ο αυτόματος έλεγχος ανεπιθύμητου περιεχομένου ολοκληρώθηκε",
    text_expired: " Η επαλήθευση Anti-Robot έληξε",
    button_restart: " Επανεκκίνηση",
    text_error: " Η επαλήθευση απέτυχε",
    button_retry: " Δοκιμάστε ξανά",
    text_fetch_error: " Αποτυχία σύνδεσης με"
  };
  // Ukrainian
  var LANG_UK = {
    text_init: "Ініціалізація...",
    text_ready: "Антиробот верифікація",
    button_start: "Натисніть, щоб розпочати верифікацію",
    text_fetching: "З’єднання",
    text_solving: "Перевірка, що ви не робот...",
    text_completed: "Я не робот",
    text_completed_sr: "Автоматична перевірка спаму завершена",
    text_expired: "Час вичерпано",
    button_restart: "Почати знову",
    text_error: "Верифікація не вдалась",
    button_retry: "Спробувати знову",
    text_fetch_error: "Не вдалось з’єднатись"
  };
  // Bulgarian
  var LANG_BG = {
    text_init: "Инициализиране...",
    text_ready: "Анти-робот проверка",
    button_start: "Щракнете, за да започнете проверката",
    text_fetching: "Предизвикателство",
    text_solving: "Проверяваме дали си човек...",
    text_completed: "Аз съм човек",
    text_completed_sr: "Автоматичната проверка за спам е завършена",
    text_expired: "Анти-Робот проверката изтече",
    button_restart: "Рестартирайте",
    text_error: "Неуспешна проверка",
    button_retry: "Опитайте пак",
    text_fetch_error: "Неуспешно свързване с"
  };
  // Czech
  var LANG_CS = {
    text_init: "Inicializace...",
    text_ready: "Ověření proti robotům",
    button_start: "Klikněte pro ověření",
    text_fetching: "Problém při načítání",
    text_solving: "Ověření, že jste člověk...",
    text_completed: "Jsem člověk",
    text_completed_sr: "Automatická kontrola spamu dokončena",
    text_expired: "Ověření proti robotům vypršelo",
    button_restart: "Restartovat",
    text_error: "Ověření se nezdařilo",
    button_retry: "Zkusit znovu",
    text_fetch_error: "Připojení se nezdařilo"
  };
  // Slovak
  var LANG_SK = {
    text_init: "Inicializácia...",
    text_ready: "Overenie proti robotom",
    button_start: "Kliknite pre overenie",
    text_fetching: "Problém pri načítaní",
    text_solving: "Overenie, že ste človek...",
    text_completed: "Som človek",
    text_completed_sr: "Automatická kontrola spamu dokončená",
    text_expired: "Overenie proti robotom vypršalo",
    button_restart: "Reštartovať",
    text_error: "Overenie sa nepodarilo",
    button_retry: "Skúsiť znova",
    text_fetch_error: "Pripojenie sa nepodarilo"
  };
  // Norwegian
  var LANG_NO = {
    text_init: " Aktiverer...",
    text_ready: "Jeg er ikke en robot",
    button_start: "Klikk for å starte verifiseringen",
    text_fetching: "Henter data",
    text_solving: "Sjekker at du er et menneske...",
    text_completed: "Jeg er et menneske",
    text_completed_sr: "Automatisk spam-sjekk fullført",
    text_expired: "Verifisering kunne ikke fullføres",
    button_restart: "Omstart",
    text_error: "Bekreftelsen mislyktes",
    button_retry: "Prøv på nytt",
    text_fetch_error: "Tilkoblingen mislyktes"
  };
  // Finnish
  var LANG_FI = {
    text_init: "Aktivoidaan...",
    text_ready: "En ole robotti",
    button_start: "Aloita vahvistus klikkaamalla",
    text_fetching: "Haetaan tietoja",
    text_solving: "Tarkistaa, että olet ihminen...",
    text_completed: "Olen ihminen",
    text_completed_sr: "Automaattinen roskapostin tarkistus suoritettu",
    text_expired: "Vahvistusta ei voitu suorittaa loppuun",
    button_restart: "Uudelleenkäynnistys",
    text_error: "Vahvistus epäonnistui",
    button_retry: "Yritä uudelleen",
    text_fetch_error: "Yhteys epäonnistui"
  };
  // Latvian
  var LANG_LV = {
    text_init: "Notiek inicializēšana...",
    text_ready: "Verifikācija, ka neesat robots",
    button_start: "Noklikšķiniet, lai sāktu verifikāciju",
    text_fetching: "Notiek drošības uzdevuma izgūšana",
    text_solving: "Notiek pārbaude, vai esat cilvēks...",
    text_completed: "Es esmu cilvēks",
    text_completed_sr: "Automātiska surogātpasta pārbaude pabeigta",
    text_expired: "Verifikācijas, ka neesat robots, derīgums beidzies",
    button_restart: "Restartēt",
    text_error: "Verifikācija neizdevās",
    button_retry: "Mēģināt vēlreiz",
    text_fetch_error: "Neizdevās izveidot savienojumu ar"
  };
  // Lithuanian
  var LANG_LT = {
    text_init: "Inicijuojama...",
    text_ready: "Patikrinimas, ar nesate robotas",
    button_start: "Spustelėkite patikrinimui pradėti",
    text_fetching: "Gavimo iššūkis",
    text_solving: "Tikrinama, ar esate žmogus...",
    text_completed: "Esu žmogus",
    text_completed_sr: "Automatinė patikra dėl pašto šiukšlių atlikta",
    text_expired: "Patikrinimas, ar nesate robotas, baigė galioti",
    button_restart: "Pradėti iš naujo",
    text_error: "Patikrinimas nepavyko",
    button_retry: "Kartoti",
    text_fetch_error: "Nepavyko prisijungti prie"
  };
  // Polish
  var LANG_PL = {
    text_init: "Inicjowanie...",
    text_ready: "Weryfikacja antybotowa",
    button_start: "Kliknij, aby rozpocząć weryfikację",
    text_fetching: "Pobieranie",
    text_solving: "Weryfikacja, czy nie jesteś robotem...",
    text_completed: "Nie jestem robotem",
    text_completed_sr: "Zakończono automatyczne sprawdzanie spamu",
    text_expired: "Weryfikacja antybotowa wygasła",
    button_restart: "Uruchom ponownie",
    text_error: "Weryfikacja nie powiodła się",
    button_retry: "Spróbuj ponownie",
    text_fetch_error: "Nie udało się połączyć z"
  };
  // Estonian
  var LANG_ET = {
    text_init: "Initsialiseerimine...",
    text_ready: "Robotivastane kinnitus",
    button_start: "Kinnitamisega alustamiseks klõpsake",
    text_fetching: "Väljakutse toomine",
    text_solving: "Kinnitatakse, et sa oled inimene...",
    text_completed: "Ma olen inimene",
    text_completed_sr: "Automaatne rämpsposti kontroll on lõpetatud",
    text_expired: "Robotivastane kinnitus aegus",
    button_restart: "Taaskäivita",
    text_error: "Kinnitamine nurjus",
    button_retry: "Proovi uuesti",
    text_fetch_error: "Ühenduse loomine nurjus"
  };
  // Croatian
  var LANG_HR = {
    text_init: "Početno postavljanje...",
    text_ready: "Provjera protiv robota",
    button_start: "Kliknite za početak provjere",
    text_fetching: "Dohvaćanje izazova",
    text_solving: "Provjeravamo jeste li čovjek...",
    text_completed: "Nisam robot",
    text_completed_sr: "Automatska provjera je završena",
    text_expired: "Vrijeme za provjeru protiv robota je isteklo",
    button_restart: "Osvježi",
    text_error: "Provjera nije uspjlela",
    button_retry: " Ponovo pokreni",
    text_fetch_error: "Nije moguće uspostaviti vezu"
  };
  // Serbian
  var LANG_SR = {
    text_init: "Pokretanje...",
    text_ready: "Anti-Robot Verifikacija",
    button_start: "Kliknite da biste započeli verifikaciju",
    text_fetching: "Učitavanje izazova",
    text_solving: "Verifikacija da ste čovek...",
    text_completed: "Ja sam čovek",
    text_completed_sr: "Automatska provera neželjene pošte je završena",
    text_expired: "Anti-Robot verifikacija je istekla",
    button_restart: "Ponovo pokrenuti",
    text_error: "Verifikacija nije uspela",
    button_retry: "Pokušajte ponovo",
    text_fetch_error: "Neuspelo povezivanje sa..."
  };
  // Slovenian
  var LANG_SL = {
    text_init: "Inicializiranje...",
    text_ready: "Preverjanje robotov",
    button_start: "Kliknite za začetek preverjanja",
    text_fetching: "Prenašanje izziva",
    text_solving: "Preverjamo, ali ste človek",
    text_completed: "Nisem robot",
    text_completed_sr: "Avtomatsko preverjanje je zaključeno",
    text_expired: "Preverjanje robotov je poteklo",
    button_restart: "Osveži",
    text_error: "Preverjanje ni uspelo",
    button_retry: "Poskusi ponovno",
    text_fetch_error: "Povezave ni bilo mogoče vzpostaviti"
  };
  // Hungarian
  var LANG_HU = {
    text_init: "Inicializálás...",
    text_ready: "Robotellenes ellenőrzés",
    button_start: "Kattintson az ellenőrzés megkezdéséhez",
    text_fetching: "Feladvány lekérése",
    text_solving: "Annak igazolása, hogy Ön nem robot...",
    text_completed: "Nem vagyok robot",
    text_completed_sr: "Automatikus spam ellenőrzés befejeződött",
    text_expired: "Robotellenes ellenőrzés lejárt",
    button_restart: "Újraindítás",
    text_error: "Az ellenőrzés nem sikerült",
    button_retry: "Próbálja újra",
    text_fetch_error: "Nem sikerült csatlakozni"
  };
  // Romanian
  var LANG_RO = {
    text_init: "Se inițializează...",
    text_ready: "Verificare anti-robot",
    button_start: "Click pentru a începe verificarea",
    text_fetching: "Downloading",
    text_solving: "Verificare că ești om...",
    text_completed: "Sunt om",
    text_completed_sr: "Verificarea automată a spam-ului a fost finalizată",
    text_expired: "Verificarea anti-robot a expirat",
    button_restart: "Restart",
    text_error: "Verificare eșuată",
    button_retry: "Reîncearcă",
    text_fetch_error: "Nu s-a putut conecta"
  };
  // Chinese
  var LANG_ZH = {
    text_init: "初始化中……",
    text_ready: "人机验证",
    button_start: "点击开始",
    text_fetching: "正在加载",
    text_solving: "人机校验中……",
    text_completed: "我不是机器人",
    text_completed_sr: "人机验证完成",
    text_expired: "验证已过期",
    button_restart: "重新开始",
    text_error: "校验失败",
    button_retry: "重试",
    text_fetch_error: "无法连接到"
  };
  // Traditional Chinese
  var LANG_ZH_TW = {
    text_init: "正在初始化……",
    text_ready: "反機器人驗證",
    button_start: "點擊開始驗證",
    text_fetching: "載入中",
    text_solving: "反機器人驗證中……",
    text_completed: "我不是機器人",
    text_completed_sr: "驗證完成",
    text_expired: "驗證超時",
    button_restart: "重新開始",
    text_error: "驗證失敗",
    button_retry: "重試",
    text_fetch_error: "無法連線到"
  };
  // Vietnamese
  var LANG_VI = {
    text_init: "Đang khởi tạo...",
    text_ready: "Xác minh chống Robot",
    button_start: "Bấm vào đây để xác minh",
    text_fetching: "Tìm nạp và xử lý thử thách",
    text_solving: "Xác minh bạn là người...",
    text_completed: "Bạn là con người",
    text_completed_sr: "Xác minh hoàn tất",
    text_expired: "Xác minh đã hết hạn",
    button_restart: "Khởi động lại",
    text_error: "Xác minh thất bại",
    button_retry: "Thử lại",
    text_fetch_error: "Không kết nối được"
  };
  // Hebrew
  var LANG_HE = {
    text_init: "בביצוע...",
    text_ready: "אימות אנוש",
    button_start: "צריך ללחוץ להתחלת האימות",
    text_fetching: "אתגר המענה בהכנה",
    text_solving: "מתבצע אימות אנוש...",
    text_completed: "אני לא רובוט",
    text_completed_sr: "בדיקת הספאם האוטומטית הסתיימה",
    text_expired: "פג תוקף אימות האנוש",
    button_restart: "להתחיל שוב",
    text_error: "אימות האנוש נכשל",
    button_retry: "לנסות שוב",
    text_fetch_error: "נכשל החיבור אל",
    rtl: true
  };
  // Thai
  var LANG_TH = {
    text_init: "การเริ่มต้น...",
    text_ready: " การตรวจสอบต่อต้านหุ่นยนต์",
    button_start: "คลิกเพื่อเริ่มการตรวจสอบ",
    text_fetching: "การดึงความท้าทาย",
    text_solving: "ยืนยันว่าคุณเป็นมนุษย์...",
    text_completed: "ฉันเป็นมนุษย์",
    text_completed_sr: "การตรวจสอบสแปมอัตโนมัติเสร็จสมบูรณ์",
    text_expired: "การตรวจสอบ ต่อต้านหุ่นยนต์ หมดอายุ",
    button_restart: "รีสตาร์ท",
    text_error: "การยืนยันล้มเหลว",
    button_retry: "ลองใหม่",
    text_fetch_error: "ไม่สามารถเชื่อมต่อได้"
  };
  // South Korean
  var LANG_KR = {
    text_init: "초기화 중",
    text_ready: "Anti-Robot 검증",
    button_start: "검증을 위해 클릭해 주세요",
    text_fetching: "검증 준비 중",
    text_solving: "검증 중",
    text_completed: "검증이 완료되었습니다",
    text_completed_sr: "자동 스팸 확인 완료",
    text_expired: "Anti-Robot 검증 만료",
    button_restart: "다시 시작합니다",
    text_error: "검증 실패",
    button_retry: "다시 시도해 주세요",
    text_fetch_error: "연결하지 못했습니다"
  };
  // Arabic
  var LANG_AR = {
    text_init: "...التهيئة",
    text_ready: "يتم التحقيق",
    button_start: "إضغط هنا للتحقيق",
    text_fetching: "تهيئة التحدي",
    text_solving: "نتحقق من أنك لست روبوتًا...",
    text_completed: "أنا لست روبوتًا",
    text_completed_sr: "تم الانتهاء من التحقق التلقائي من البريد العشوائي",
    text_expired: "انتهت صلاحية التحقق",
    button_restart: "إعادة تشغيل",
    text_error: "فشل التحقق",
    button_retry: "ابدأ مرة أخرى",
    text_fetch_error: "مشكلة في الاتصال مع"
  };
  var localizations = {
    en: LANG_EN,
    de: LANG_DE,
    nl: LANG_NL,
    fr: LANG_FR,
    it: LANG_IT,
    pt: LANG_PT,
    es: LANG_ES,
    ca: LANG_CA,
    ja: LANG_JA,
    da: LANG_DA,
    ru: LANG_RU,
    sv: LANG_SV,
    tr: LANG_TR,
    el: LANG_EL,
    uk: LANG_UK,
    bg: LANG_BG,
    cs: LANG_CS,
    sk: LANG_SK,
    no: LANG_NO,
    fi: LANG_FI,
    lv: LANG_LV,
    lt: LANG_LT,
    pl: LANG_PL,
    et: LANG_ET,
    hr: LANG_HR,
    sr: LANG_SR,
    sl: LANG_SL,
    hu: LANG_HU,
    ro: LANG_RO,
    zh: LANG_ZH,
    zh_tw: LANG_ZH_TW,
    vi: LANG_VI,
    he: LANG_HE,
    th: LANG_TH,
    kr: LANG_KR,
    ar: LANG_AR,
    // alternative language codes
    nb: LANG_NO
  };
  function createDiagnosticsBuffer(solverID, timeToSolved) {
    var arr = new Uint8Array(3);
    var view = new DataView(arr.buffer);
    view.setUint8(0, solverID);
    view.setUint16(1, timeToSolved);
    return arr;
  }
  var workerString = "!function(){function A(r){return A=\"function\"==typeof Symbol&&\"symbol\"==typeof Symbol.iterator?function(A){return typeof A}:function(A){return A&&\"function\"==typeof Symbol&&A.constructor===Symbol&&A!==Symbol.prototype?\"symbol\":typeof A},A(r)}function r(A,r){(null==r||r>A.length)&&(r=A.length);for(var t=0,n=Array(r);t<r;t++)n[t]=A[t];return n}function t(A,r){for(var t=0;t<r.length;t++){var n=r[t];n.enumerable=n.enumerable||!1,n.configurable=!0,\"value\"in n&&(n.writable=!0),Object.defineProperty(A,e(n.key),n)}}function n(A,r,n){return r&&t(A.prototype,r),n&&t(A,n),Object.defineProperty(A,\"prototype\",{writable:!1}),A}function e(r){var t=function(r){if(\"object\"!=A(r)||!r)return r;var t=r[Symbol.toPrimitive];if(void 0!==t){var n=t.call(r,\"string\");if(\"object\"!=A(n))return n;throw new TypeError(\"@@toPrimitive must return a primitive value.\")}return String(r)}(r);return\"symbol\"==A(t)?t:t+\"\"}!function(){\"use strict\";var A,r=[];function t(){for(;r.length;)r[0](),r.shift()}function n(A){this.a=e,this.b=void 0,this.f=[];var r=this;try{A((function(A){i(r,A)}),(function(A){g(r,A)}))}catch(t){g(r,t)}}A=function(){setTimeout(t)};var e=2;function o(A){return new n((function(r){r(A)}))}function i(A,r){if(A.a==e){if(r==A)throw new TypeError;var t=!1;try{var n=r&&r.then;if(null!=r&&\"object\"==typeof r&&\"function\"==typeof n)return void n.call(r,(function(r){t||i(A,r),t=!0}),(function(r){t||g(A,r),t=!0}))}catch(o){return void(t||g(A,o))}A.a=0,A.b=r,I(A)}}function g(A,r){if(A.a==e){if(r==A)throw new TypeError;A.a=1,A.b=r,I(A)}}function I(t){!function(t){r.push(t),1==r.length&&A()}((function(){if(t.a!=e)for(;t.f.length;){var A=(o=t.f.shift())[0],r=o[1],n=o[2],o=o[3];try{0==t.a?n(\"function\"==typeof A?A.call(void 0,t.b):t.b):1==t.a&&(\"function\"==typeof r?n(r.call(void 0,t.b)):o(t.b))}catch(i){o(i)}}}))}n.prototype.g=function(A){return this.c(void 0,A)},n.prototype.c=function(A,r){var t=this;return new n((function(n,e){t.f.push([A,r,n,e]),I(t)}))},self.Promise||(self.Promise=n,self.Promise.resolve=o,self.Promise.reject=function(A){return new n((function(r,t){t(A)}))},self.Promise.race=function(A){return new n((function(r,t){for(var n=0;n<A.length;n+=1)o(A[n]).c(r,t)}))},self.Promise.all=function(A){return new n((function(r,t){function n(t){return function(n){i[t]=n,(e+=1)==A.length&&r(i)}}var e=0,i=[];0==A.length&&r(i);for(var g=0;g<A.length;g+=1)o(A[g]).c(n(g),t)}))},self.Promise.prototype.then=n.prototype.c,self.Promise.prototype.catch=n.prototype.g)}(),function(){\"use strict\";for(var A=\"=\".charCodeAt(0),t=new Uint8Array(256),e=0;e<64;e++)t[\"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/\".charCodeAt(e)]=e;var o=n((function A(r){!function(A,r){if(!(A instanceof r))throw new TypeError(\"Cannot call a class as a function\")}(this,A),this.b=new Uint8Array(128),this.h=new Uint32Array(16),this.t=0,this.c=0,this.v=new Uint32Array(32),this.m=new Uint32Array(32),this.outlen=r}));function i(A,r){return A[r]^A[r+1]<<8^A[r+2]<<16^A[r+3]<<24}function g(A,r,t,n,e,o,i,g){var I,C,Q,c=r[i],a=r[i+1],f=r[g],u=r[g+1],B=A[t],s=A[t+1],E=A[n],l=A[n+1],h=A[e],w=A[e+1],y=A[o],v=A[o+1];Q=v^(s=(s=s+l+((B&E|(B|E)&~(I=B+E))>>>31))+a+(((B=I)&c|(B|c)&~(I=B+c))>>>31)),E=(Q=(l=(Q=l^(w=w+(v=C=y^(B=I))+((h&(y=Q)|(h|y)&~(I=h+y))>>>31)))>>>24^(C=E^(h=I))<<8)^(w=w+(v=(Q=v^(s=(s=s+l+((B&(E=C>>>24^Q<<8)|(B|E)&~(I=B+E))>>>31))+u+(((B=I)&f|(B|f)&~(I=B+f))>>>31)))>>>16^(C=y^(B=I))<<16)+((h&(y=C>>>16^Q<<16)|(h|y)&~(I=h+y))>>>31)))>>>31^(C=E^(h=I))<<1,l=C>>>31^Q<<1,A[t]=B,A[t+1]=s,A[n]=E,A[n+1]=l,A[e]=h,A[e+1]=w,A[o]=y,A[o+1]=v}var I,C,Q=[4089235720,1779033703,2227873595,3144134277,4271175723,1013904242,1595750129,2773480762,2917565137,1359893119,725511199,2600822924,4215389547,528734635,327033209,1541459225],c=[0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,28,20,8,16,18,30,26,12,2,24,0,4,22,14,10,6,22,16,24,0,10,4,30,26,20,28,6,12,14,2,18,8,14,18,6,2,26,24,22,28,4,12,10,20,8,0,30,16,18,0,10,14,4,8,20,30,28,2,22,24,12,16,6,26,4,24,12,20,0,22,16,6,8,26,14,10,30,28,2,18,24,10,2,30,28,26,8,20,0,14,12,6,18,4,16,22,26,22,14,28,24,2,6,18,10,0,30,8,16,12,4,20,12,30,28,18,22,6,0,16,24,4,26,14,2,8,20,10,20,4,16,8,14,12,2,10,30,22,18,28,6,24,26,0,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,28,20,8,16,18,30,26,12,2,24,0,4,22,14,10,6];function a(A,r){for(var t=A.v,n=A.m,e=0;e<16;e++)t[e]=A.h[e],t[e+16]=Q[e];t[24]=t[24]^A.t,t[25]=t[25]^A.t/4294967296,r&&(t[28]=~t[28],t[29]=~t[29]);for(var o=0;o<32;o++)n[o]=i(A.b,4*o);for(var I=0;I<12;I++)g(t,n,0,8,16,24,c[16*I+0],c[16*I+1]),g(t,n,2,10,18,26,c[16*I+2],c[16*I+3]),g(t,n,4,12,20,28,c[16*I+4],c[16*I+5]),g(t,n,6,14,22,30,c[16*I+6],c[16*I+7]),g(t,n,0,10,20,30,c[16*I+8],c[16*I+9]),g(t,n,2,12,22,24,c[16*I+10],c[16*I+11]),g(t,n,4,14,16,26,c[16*I+12],c[16*I+13]),g(t,n,6,8,18,28,c[16*I+14],c[16*I+15]);for(var C=0;C<16;C++)A.h[C]=A.h[C]^t[C]^t[C+16]}function f(A,r){for(var t=0;t<16;t++)A.h[t]=Q[t];A.b.set(r),A.h[0]^=16842752^A.outlen}function u(){return new Promise((function(A){return A((function(A,r){var t=function(A,r,t){if(128!=A.length)throw Error(\"Invalid input\");var n=A.buffer,e=new DataView(n),i=new o(32);i.t=128;for(var g=e.getUint32(124,!0),I=g+t,C=g;C<I;C++)if(e.setUint32(124,C,!0),f(i,A),a(i,!0),i.h[0]<r)return 0==ASC_TARGET?new Uint8Array(i.h.buffer):Uint8Array.wrap(i.h.buffer);return new Uint8Array(0)}(A,r,arguments.length>2&&void 0!==arguments[2]?arguments[2]:4294967295);return[A,t]}))}))}Uint8Array.prototype.slice||Object.defineProperty(Uint8Array.prototype,\"slice\",{value:function(A,r){return new Uint8Array(Array.prototype.slice.call(this,A,r))}}),self.ASC_TARGET=0;var B=new Promise((function(A){return C=A}));self.onerror=function(A){self.postMessage({type:\"error\",message:JSON.stringify(A)})},self.onmessage=function(n){return new Promise((function(e,o){var i;i=n.data;var g=function(){try{return e()}catch(A){return o(A)}},Q=function(A){try{return setTimeout((function(){throw A})),g()}catch(r){return o(r)}};try{var c=function(){return g()};if(\"solver\"!==i.type){var a,f,s,E,l=function(){return c.call(this)};return\"start\"===i.type?Promise.resolve(B).then(function(A){try{a=A,self.postMessage({type:\"started\"}),f=0;for(var t=0;t<256;t++){i.puzzleSolverInput[123]=t;var n=function(A){if(Array.isArray(A))return A}(o=a(i.puzzleSolverInput,i.threshold))||function(A){var r=null==A?null:\"undefined\"!=typeof Symbol&&A[Symbol.iterator]||A[\"@@iterator\"];if(null!=r){var t,n,e,o,i=[],g=!0,I=!1;try{for(e=(r=r.call(A)).next;!(g=(t=e.call(r)).done)&&(i.push(t.value),2!==i.length);g=!0);}catch(A){I=!0,n=A}finally{try{if(!g&&null!=r.return&&(o=r.return(),Object(o)!==o))return}finally{if(I)throw n}}return i}}(o)||function(A){if(A){if(\"string\"==typeof A)return r(A,2);var t={}.toString.call(A).slice(8,-1);return\"Object\"===t&&A.constructor&&(t=A.constructor.name),\"Map\"===t||\"Set\"===t?Array.from(A):\"Arguments\"===t||/^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(t)?r(A,2):void 0}}(o)||function(){throw new TypeError(\"Invalid attempt to destructure non-iterable instance.\\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.\")}(),e=n[0];if(0!==n[1].length){s=e;break}console.warn(\"FC: Internal error or no solution found\"),f+=Math.pow(2,32)-1}return E=new DataView(s.slice(-4).buffer),f+=E.getUint32(0,!0),self.postMessage({type:\"done\",solution:s.slice(-8),h:f,puzzleIndex:i.puzzleIndex,puzzleNumber:i.puzzleNumber}),l.call(this)}catch(g){return Q(g)}var o}.bind(this),Q):l.call(this)}var h=function(){return self.postMessage({type:\"ready\",solver:I}),c.call(this)};if(i.forceJS)return I=1,Promise.resolve(u()).then(function(A){try{return C(A),h.call(this)}catch(r){return Q(r)}}.bind(this),Q);var w=function(){try{return h.call(this)}catch(A){return Q(A)}}.bind(this),y=function(A){try{return console.log(\"FriendlyCaptcha failed to initialize WebAssembly, falling back to Javascript solver: \"+A.toString()),I=1,Promise.resolve(u()).then((function(A){try{return C(A),w()}catch(r){return Q(r)}}),Q)}catch(r){return Q(r)}};try{var v;return I=2,v=WebAssembly.compile(function(r){var n=3285;r.charCodeAt(4379)===A&&n--,r.charCodeAt(4378)===A&&n--;for(var e=new Uint8Array(n),o=0,i=0;o<4380;o+=4){var g=t[r.charCodeAt(o+0)],I=t[r.charCodeAt(o+1)],C=t[r.charCodeAt(o+2)],Q=t[r.charCodeAt(o+3)];e[i++]=g<<2|I>>4,e[i++]=(15&I)<<4|C>>2,e[i++]=(3&C)<<6|63&Q}return e}(\"AGFzbQEAAAABKghgAABgAn9/AGADf39/AX9gAX8AYAR/f39/AGAAAX9gAX8Bf2ACf38BfwINAQNlbnYFYWJvcnQABAMMCwcGAwAAAQIFAQIABQMBAAEGFgR/AUEAC38BQQALfwBBAwt/AEHgDAsHbgkGbWVtb3J5AgAHX19hbGxvYwABCF9fcmV0YWluAAIJX19yZWxlYXNlAAMJX19jb2xsZWN0AAQHX19yZXNldAAFC19fcnR0aV9iYXNlAwMNVWludDhBcnJheV9JRAMCDHNvbHZlQmxha2UyYgAKCAELCvQSC5IBAQV/IABB8P///wNLBEAACyMBQRBqIgQgAEEPakFwcSICQRAgAkEQSxsiBmoiAj8AIgVBEHQiA0sEQCAFIAIgA2tB//8DakGAgHxxQRB2IgMgBSADShtAAEEASARAIANAAEEASARAAAsLCyACJAEgBEEQayICIAY2AgAgAkEBNgIEIAIgATYCCCACIAA2AgwgBAsEACAACwMAAQsDAAELBgAjACQBC7sCAQF/AkAgAUUNACAAQQA6AAAgACABakEEayICQQA6AAMgAUECTQ0AIABBADoAASAAQQA6AAIgAkEAOgACIAJBADoAASABQQZNDQAgAEEAOgADIAJBADoAACABQQhNDQAgAEEAIABrQQNxIgJqIgBBADYCACAAIAEgAmtBfHEiAmpBHGsiAUEANgIYIAJBCE0NACAAQQA2AgQgAEEANgIIIAFBADYCECABQQA2AhQgAkEYTQ0AIABBADYCDCAAQQA2AhAgAEEANgIUIABBADYCGCABQQA2AgAgAUEANgIEIAFBADYCCCABQQA2AgwgACAAQQRxQRhqIgFqIQAgAiABayEBA0AgAUEgTwRAIABCADcDACAAQgA3AwggAEIANwMQIABCADcDGCABQSBrIQEgAEEgaiEADAELCwsLcgACfyAARQRAQQxBAhABIQALIAALQQA2AgAgAEEANgIEIABBADYCCCABQfD///8DIAJ2SwRAQcAKQfAKQRJBORAAAAsgASACdCIBQQAQASICIAEQBiAAKAIAGiAAIAI2AgAgACACNgIEIAAgATYCCCAAC88BAQJ/QaABQQAQASIAQQxBAxABQYABQQAQBzYCACAAQQxBBBABQQhBAxAHNgIEIABCADcDCCAAQQA2AhAgAEIANwMYIABC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\")),Promise.resolve(v).then((function(A){try{return Promise.resolve(function(A){return new Promise((function(r,t){var n,e,o;return Promise.resolve(function(A){return new Promise((function(r,t){var n,e;return n={env:{abort:function(){throw Error(\"Wasm aborted\")}}},Promise.resolve(WebAssembly.instantiate(A,n)).then((function(A){try{return e=function(A){var r={},t=A.exports,n=t.memory,e=t.__alloc,o=t.__retain,i=t.__rtti_base||-1;return r.__allocArray=function(A,r){var t=function(A){return new Uint32Array(n.buffer)[(i+4>>>2)+2*A]}(A),g=31-Math.clz32(t>>>6&31),I=r.length,C=e(I<<g,0),Q=e(12,A),c=new Uint32Array(n.buffer);c[Q+0>>>2]=o(C),c[Q+4>>>2]=C,c[Q+8>>>2]=I<<g;var a=n.buffer,f=new Uint8Array(a);if(16384&t)for(var u=0;u<I;++u)f[(C>>>g)+u]=o(r[u]);else f.set(r,C>>>g);return Q},r.__getUint8Array=function(A){var r=new Uint32Array(n.buffer),t=r[A+4>>>2];return new Uint8Array(n.buffer,t,r[t-4>>>2]>>>0)},function(A){var r=arguments.length>1&&void 0!==arguments[1]?arguments[1]:{},t=A.__argumentsLength?function(r){A.__argumentsLength.value=r}:A.__setArgumentsLength||A.__setargc||function(){return{}},n=function(){if(!Object.prototype.hasOwnProperty.call(A,e))return 1;var n=A[e],o=e.split(\".\")[0];\"function\"==typeof n&&n!==t?(r[o]=function(){return t(arguments.length),n.apply(void 0,arguments)}).original=n:r[o]=n};for(var e in A)n();return r}(t,r)}(A),r({exports:e})}catch(n){return t(n)}}),t)}))}(A)).then((function(A){try{return e=(n=A).exports.__retain(n.exports.__allocArray(n.exports.Uint8Array_ID,new Uint8Array(128))),o=n.exports.__getUint8Array(e),r((function(A,r){var t=arguments.length>2&&void 0!==arguments[2]?arguments[2]:4294967295;o.set(A);var i=n.exports.solveBlake2b(e,r,t);o=n.exports.__getUint8Array(e);var g=n.exports.__getUint8Array(i);return n.exports.__release(i),[o,g]}))}catch(i){return t(i)}}),t)}))}(A)).then((function(A){try{return C(A),w()}catch(r){return y(r)}}),y)}catch(r){return y(r)}}),y)}catch(p){y(p)}}catch(p){Q(p)}}))}}()}(\"undefined\"==typeof frcWorker?frcWorker={}:frcWorker);";

  // Defensive init to make it easier to integrate with Gatsby and friends.
  var URL;
  if (typeof window !== "undefined") {
    URL = window.URL || window.webkitURL;
  }
  var WorkerGroup = /*#__PURE__*/function () {
    function WorkerGroup() {
      _classCallCheck(this, WorkerGroup);
      this.workers = [];
      this.puzzleNumber = 0;
      this.numPuzzles = 0;
      this.threshold = 0;
      this.startTime = 0;
      this.progress = 0;
      this.totalHashes = 0;
      this.puzzleSolverInputs = [];
      // The index of the next puzzle
      this.puzzleIndex = 0;
      this.solutionBuffer = new Uint8Array(0);
      // initialize some value, so ts is happy
      this.solverType = 1;
      this.readyPromise = new Promise(function () {});
      this.readyCount = 0;
      this.startCount = 0;
      this.progressCallback = function () {
        return 0;
      };
      this.readyCallback = function () {
        return 0;
      };
      this.startedCallback = function () {
        return 0;
      };
      this.doneCallback = function () {
        return 0;
      };
      this.errorCallback = function () {
        return 0;
      };
    }
    return _createClass(WorkerGroup, [{
      key: "init",
      value: function init() {
        var _this = this;
        this.terminateWorkers();
        this.progress = 0;
        this.totalHashes = 0;
        var setReady;
        this.readyPromise = new Promise(function (resolve) {
          return setReady = resolve;
        });
        this.readyCount = 0;
        this.startCount = 0;
        // Setup four workers for now - later we could calculate this depending on the device
        this.workers = new Array(4);
        var workerBlob = new Blob([workerString], {
          type: "text/javascript"
        });
        var _loop = function _loop(_i4) {
          _this.workers[_i4] = new Worker(URL.createObjectURL(workerBlob));
          _this.workers[_i4].onerror = function (e) {
            return _this.errorCallback(e);
          };
          _this.workers[_i4].onmessage = function (e) {
            var data = e.data;
            if (!data) return;
            if (data.type === "ready") {
              _this.readyCount++;
              _this.solverType = data.solver;
              // We are ready, when all workers are ready
              if (_this.readyCount == _this.workers.length) {
                setReady();
                _this.readyCallback();
              }
            } else if (data.type === "started") {
              _this.startCount++;
              // We started, when the first worker starts working
              if (_this.startCount == 1) {
                _this.startTime = Date.now();
                _this.startedCallback();
              }
            } else if (data.type === "done") {
              if (data.puzzleNumber !== _this.puzzleNumber) return; // solution belongs to a previous puzzle
              if (_this.puzzleIndex < _this.puzzleSolverInputs.length) {
                _this.workers[_i4].postMessage({
                  type: "start",
                  puzzleSolverInput: _this.puzzleSolverInputs[_this.puzzleIndex],
                  threshold: _this.threshold,
                  puzzleIndex: _this.puzzleIndex,
                  puzzleNumber: _this.puzzleNumber
                });
                _this.puzzleIndex++;
              }
              _this.progress++;
              _this.totalHashes += data.h;
              _this.progressCallback({
                n: _this.numPuzzles,
                h: _this.totalHashes,
                t: (Date.now() - _this.startTime) / 1000,
                i: _this.progress
              });
              _this.solutionBuffer.set(data.solution, data.puzzleIndex * 8);
              // We are done, when all puzzles have been solved
              if (_this.progress == _this.numPuzzles) {
                var totalTime = (Date.now() - _this.startTime) / 1000;
                _this.doneCallback({
                  solution: _this.solutionBuffer,
                  h: _this.totalHashes,
                  t: totalTime,
                  diagnostics: createDiagnosticsBuffer(_this.solverType, totalTime),
                  solver: _this.solverType
                });
              }
            } else if (data.type === "error") {
              _this.errorCallback(data);
            }
          };
        };
        for (var _i4 = 0; _i4 < this.workers.length; _i4++) {
          _loop(_i4);
        }
      }
    }, {
      key: "setupSolver",
      value: function setupSolver() {
        var forceJS = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : false;
        var msg = {
          type: "solver",
          forceJS: forceJS
        };
        for (var _i5 = 0; _i5 < this.workers.length; _i5++) {
          this.workers[_i5].postMessage(msg);
        }
      }
    }, {
      key: "start",
      value: function start(puzzle) {
        return new Promise(function ($return, $error) {
          return Promise.resolve(this.readyPromise).then(function ($await_11) {
            try {
              this.puzzleSolverInputs = getPuzzleSolverInputs(puzzle.buffer, puzzle.n);
              this.solutionBuffer = new Uint8Array(8 * puzzle.n);
              this.numPuzzles = puzzle.n;
              this.threshold = puzzle.threshold;
              this.puzzleIndex = 0;
              this.puzzleNumber++;
              for (var _i6 = 0; _i6 < this.workers.length; _i6++) {
                if (this.puzzleIndex === this.puzzleSolverInputs.length) break;
                this.workers[_i6].postMessage({
                  type: "start",
                  puzzleSolverInput: this.puzzleSolverInputs[_i6],
                  threshold: this.threshold,
                  puzzleIndex: this.puzzleIndex,
                  puzzleNumber: this.puzzleNumber
                });
                this.puzzleIndex++;
              }
              return $return();
            } catch ($boundEx) {
              return $error($boundEx);
            }
          }.bind(this), $error);
        }.bind(this));
      }
    }, {
      key: "terminateWorkers",
      value: function terminateWorkers() {
        if (this.workers.length == 0) return;
        for (var _i7 = 0; _i7 < this.workers.length; _i7++) {
          this.workers[_i7].terminate();
        }
        this.workers = [];
      }
    }]);
  }();
  var PUZZLE_ENDPOINT_URL = "https://api.friendlycaptcha.com/api/v1/puzzle";
  var WidgetInstance = /*#__PURE__*/function () {
    function WidgetInstance(element) {
      var options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
      _classCallCheck(this, WidgetInstance);
      this.workerGroup = new WorkerGroup();
      /**
       * The captcha has been succesfully solved.
       */
      this.valid = false;
      /**
       * Some errors may cause a need for the (worker) to be reinitialized. If this is
       * true `init` will be called again when start is called.
       */
      this.needsReInit = false;
      /**
       * Start() has been called at least once ever.
       */
      this.hasBeenStarted = false;
      this.hasBeenDestroyed = false;
      this.opts = Object.assign({
        forceJSFallback: false,
        skipStyleInjection: false,
        startMode: "focus",
        puzzleEndpoint: element.dataset["puzzleEndpoint"] || PUZZLE_ENDPOINT_URL,
        startedCallback: function startedCallback() {
          return 0;
        },
        readyCallback: function readyCallback() {
          return 0;
        },
        doneCallback: function doneCallback() {
          return 0;
        },
        errorCallback: function errorCallback() {
          return 0;
        },
        sitekey: element.dataset["sitekey"] || "",
        language: element.dataset["lang"] || "en",
        solutionFieldName: element.dataset["solutionFieldName"] || "frc-captcha-solution",
        styleNonce: null
      }, options);
      this.e = element;
      this.e.friendlyChallengeWidget = this;
      this.loadLanguage();
      // @ts-ignore Ignore is required as TS thinks that `this.lang` is not assigned yet, but it happens in `this.loadLanguage()` above.
      element.innerText = this.lang.text_init;
      if (!this.opts.skipStyleInjection) {
        injectStyle(this.opts.styleNonce);
      }
      this.init(this.opts.startMode === "auto" || this.e.dataset["start"] === "auto");
    }
    return _createClass(WidgetInstance, [{
      key: "init",
      value: function init(forceStart) {
        var _this2 = this;
        if (this.hasBeenDestroyed) {
          console.error("FriendlyCaptcha widget has been destroyed using destroy(), it can not be used anymore.");
          return;
        }
        this.initWorkerGroup();
        if (forceStart) {
          this.start();
        } else if (this.e.dataset["start"] !== "none" && (this.opts.startMode === "focus" || this.e.dataset["start"] === "focus")) {
          var form = findParentFormElement(this.e);
          if (form) {
            executeOnceOnFocusInEvent(form, function () {
              return _this2.start();
            });
          } else {
            console.log("FriendlyCaptcha div seems not to be contained in a form, autostart will not work");
          }
        }
      }
      /**
       * Loads the configured language, or a language passed to this function.
       * Note that only the next update will be in the new language, consider calling `reset()` after switching languages.
       */
    }, {
      key: "loadLanguage",
      value: function loadLanguage(lang) {
        if (lang !== undefined) {
          this.opts.language = lang;
        } else if (this.e.dataset["lang"]) {
          this.opts.language = this.e.dataset["lang"];
        }
        if (typeof this.opts.language === "string") {
          var langCode = this.opts.language.toLowerCase();
          var l = localizations[langCode];
          if (l === undefined && langCode[2] === "-") {
            // Language has a locale '-' separator, remove it and try again
            langCode = langCode.substring(0, 2);
            l = localizations[langCode];
          }
          if (l === undefined) {
            console.error('FriendlyCaptcha: language "' + this.opts.language + '" not found.');
            // Fall back to English
            l = localizations.en;
          }
          this.lang = l;
        } else {
          // We assign to a copy of the English language localization, so that any missing values will be English
          this.lang = Object.assign(Object.assign({}, localizations.en), this.opts.language);
        }
      }
      /**
       * Add a listener to the button that calls `this.start` on click.
       */
    }, {
      key: "makeButtonStart",
      value: function makeButtonStart() {
        var _this3 = this;
        var b = this.e.querySelector("button");
        if (b) {
          b.addEventListener("click", function (e) {
            return _this3.start();
          }, {
            once: true,
            passive: true
          });
          b.addEventListener("touchstart", function (e) {
            return _this3.start();
          }, {
            once: true,
            passive: true
          });
        }
      }
    }, {
      key: "onWorkerError",
      value: function onWorkerError(e) {
        this.hasBeenStarted = false;
        this.needsReInit = true;
        if (this.expiryTimeout) clearTimeout(this.expiryTimeout);
        console.error("[FRC]", e);
        this.e.innerHTML = getErrorHTML(this.opts.solutionFieldName, this.lang, "Background worker error " + e.message);
        this.makeButtonStart();
        // Just out of precaution
        this.opts.forceJSFallback = true;
      }
    }, {
      key: "initWorkerGroup",
      value: function initWorkerGroup() {
        var _this4 = this;
        this.workerGroup.progressCallback = function (progress) {
          updateProgressBar(_this4.e, progress);
        };
        this.workerGroup.readyCallback = function () {
          _this4.e.innerHTML = getReadyHTML(_this4.opts.solutionFieldName, _this4.lang);
          _this4.makeButtonStart();
          _this4.opts.readyCallback();
        };
        this.workerGroup.startedCallback = function () {
          _this4.e.innerHTML = getRunningHTML(_this4.opts.solutionFieldName, _this4.lang);
          _this4.opts.startedCallback();
        };
        this.workerGroup.doneCallback = function (data) {
          var solutionPayload = _this4.handleDone(data);
          _this4.opts.doneCallback(solutionPayload);
          var callback = _this4.e.dataset["callback"];
          if (callback) {
            window[callback](solutionPayload);
          }
        };
        this.workerGroup.errorCallback = function (e) {
          _this4.onWorkerError(e);
        };
        this.workerGroup.init();
        this.workerGroup.setupSolver(this.opts.forceJSFallback);
      }
    }, {
      key: "expire",
      value: function expire() {
        this.hasBeenStarted = false;
        // Node.isConnected will be undefined in older browsers
        if (this.e.isConnected !== false) {
          this.e.innerHTML = getExpiredHTML(this.opts.solutionFieldName, this.lang);
          this.makeButtonStart();
        }
      }
    }, {
      key: "start",
      value: function start() {
        return new Promise(function ($return, $error) {
          var sitekey;
          if (this.hasBeenDestroyed) {
            console.error("Can not start FriendlyCaptcha widget which has been destroyed");
            return $return();
          }
          if (this.hasBeenStarted) {
            console.warn("Can not start FriendlyCaptcha widget which has already been started");
            return $return();
          }
          sitekey = this.opts.sitekey || this.e.dataset["sitekey"];
          if (!sitekey) {
            console.error("FriendlyCaptcha: sitekey not set on frc-captcha element");
            this.e.innerHTML = getErrorHTML(this.opts.solutionFieldName, this.lang, "Website problem: sitekey not set", false);
            return $return();
          }
          if (isHeadless()) {
            this.e.innerHTML = getErrorHTML(this.opts.solutionFieldName, this.lang, "Browser check failed, try a different browser", false, true);
            return $return();
          }
          if (this.needsReInit) {
            this.needsReInit = false;
            this.init(true);
            return $return();
          }
          this.hasBeenStarted = true;
          var $Try_3_Post = function () {
            try {
              return Promise.resolve(this.workerGroup.start(this.puzzle)).then(function ($await_12) {
                try {
                  return $return();
                } catch ($boundEx) {
                  return $error($boundEx);
                }
              }, $error);
            } catch ($boundEx) {
              return $error($boundEx);
            }
          }.bind(this);
          var $Try_3_Catch = function (e) {
            try {
              {
                console.error("[FRC]", e);
                this.hasBeenStarted = false;
                if (this.expiryTimeout) clearTimeout(this.expiryTimeout);
                this.e.innerHTML = getErrorHTML(this.opts.solutionFieldName, this.lang, e.message);
                this.makeButtonStart();
                var code;
                code = "error_getting_puzzle";
                this.opts.errorCallback({
                  code: code,
                  description: e.toString(),
                  error: e
                });
                var callback;
                callback = this.e.dataset["callback-error"];
                if (callback) {
                  window[callback](this);
                }
                return $return();
              }
            } catch ($boundEx) {
              return $error($boundEx);
            }
          }.bind(this);
          try {
            this.e.innerHTML = getFetchingHTML(this.opts.solutionFieldName, this.lang);
            return Promise.resolve(getPuzzle(this.opts.puzzleEndpoint, sitekey, this.lang)).then(function ($await_13) {
              var _this5 = this;
              try {
                this.puzzle = decodeBase64Puzzle($await_13);
                if (this.expiryTimeout) clearTimeout(this.expiryTimeout);
                this.expiryTimeout = setTimeout(function () {
                  return _this5.expire();
                }, this.puzzle.expiry - 30000); // 30s grace
                return $Try_3_Post();
              } catch ($boundEx) {
                return $Try_3_Catch($boundEx);
              }
            }.bind(this), $Try_3_Catch);
          } catch (e) {
            $Try_3_Catch(e);
          }
        }.bind(this));
      }
      /**
       * This is to be called when the puzzle has been succesfully completed.
       * Here the hidden field gets updated with the solution.
       * @param data message from the webworker
       */
    }, {
      key: "handleDone",
      value: function handleDone(data) {
        this.valid = true;
        var puzzleSolutionMessage = "".concat(this.puzzle.signature, ".").concat(this.puzzle.base64, ".").concat(encode(data.solution), ".").concat(encode(data.diagnostics));
        this.e.innerHTML = getDoneHTML(this.opts.solutionFieldName, this.lang, puzzleSolutionMessage, data);
        // this.worker = null; // This literally crashes very old browsers..
        this.needsReInit = true;
        return puzzleSolutionMessage;
      }
      /**
       * Cleans up the widget entirely, removing any DOM elements and terminating any background workers.
       * After it is destroyed it can no longer be used for any purpose.
       */
    }, {
      key: "destroy",
      value: function destroy() {
        this.workerGroup.terminateWorkers();
        this.needsReInit = false;
        this.hasBeenStarted = false;
        if (this.expiryTimeout) clearTimeout(this.expiryTimeout);
        if (this.e) {
          this.e.remove();
          // eslint-disable-next-line @typescript-eslint/ban-ts-ignore
          // @ts-ignore
          delete this.e;
        }
        this.hasBeenDestroyed = true;
      }
      /**
       * Resets the widget to the initial state.
       * This is useful in situations where the page does not refresh when you submit and the form may be re-submitted again
       */
    }, {
      key: "reset",
      value: function reset() {
        if (this.hasBeenDestroyed) {
          console.error("FriendlyCaptcha widget has been destroyed, it can not be used anymore");
          return;
        }
        this.workerGroup.terminateWorkers();
        this.needsReInit = false;
        this.hasBeenStarted = false;
        if (this.expiryTimeout) clearTimeout(this.expiryTimeout);
        this.init(this.opts.startMode === "auto" || this.e.dataset["start"] === "auto");
      }
    }]);
  }();
  window.friendlyChallenge = {
    WidgetInstance: WidgetInstance
  };
  function setup() {
    var autoWidget = window.friendlyChallenge.autoWidget;
    var elements = findCaptchaElements();
    for (var index = 0; index < elements.length; index++) {
      var hElement = elements[index];
      if (hElement && !hElement.dataset["attached"]) {
        autoWidget = new WidgetInstance(hElement);
        // We set the "data-attached" attribute so we don't attach to the same element twice.
        hElement.dataset["attached"] = "1";
      }
    }
    window.friendlyChallenge.autoWidget = autoWidget;
  }
  if (document.readyState !== "loading") {
    setup();
  } else {
    document.addEventListener("DOMContentLoaded", setup);
  }
})();
