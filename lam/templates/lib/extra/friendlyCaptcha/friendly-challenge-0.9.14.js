var css = '.frc-captcha *{margin:0;padding:0;border:0;text-align:initial;border-radius:px;filter:none!important;transition:none!important;font-weight:400;font-size:14px;line-height:1.2;text-decoration:none;background-color:initial;color:#222}.frc-captcha{position:relative;min-width:250px;max-width:312px;border:1px solid #f4f4f4;padding-bottom:12px;background-color:#fff}.frc-captcha b{font-weight:700}.frc-container{display:flex;align-items:center;min-height:52px}.frc-icon{fill:#222;stroke:#222;flex-shrink:0;margin:8px 8px 0}.frc-icon.frc-warning{fill:#c00}.frc-success .frc-icon{animation:1s ease-in both frc-fade-in}.frc-content{white-space:nowrap;display:flex;flex-direction:column;margin:4px 6px 0 0;overflow-x:auto;flex-grow:1}.frc-banner{position:absolute;bottom:0;right:6px;line-height:1}.frc-banner *{font-size:10px;opacity:.8;text-decoration:none}.frc-progress{-webkit-appearance:none;-moz-appearance:none;appearance:none;margin:3px 0;height:4px;border:none;background-color:#eee;color:#222;width:100%;transition:.5s linear}.frc-progress::-webkit-progress-bar{background:#eee}.frc-progress::-webkit-progress-value{background:#222}.frc-progress::-moz-progress-bar{background:#222}.frc-button{cursor:pointer;padding:2px 6px;background-color:#f1f1f1;border:1px solid transparent;text-align:center;font-weight:600;text-transform:none}.frc-button:focus{border:1px solid #333}.frc-button:hover{background-color:#ddd}.frc-captcha-solution{display:none}.frc-err-url{text-decoration:underline;font-size:.9em}.frc-rtl{direction:rtl}.frc-rtl .frc-content{margin:4px 0 0 6px}.frc-banner.frc-rtl{left:6px;right:auto}.dark.frc-captcha{color:#fff;background-color:#222;border-color:#333}.dark.frc-captcha *{color:#fff}.dark.frc-captcha button{background-color:#444}.dark .frc-icon{fill:#fff;stroke:#fff}.dark .frc-progress{background-color:#444}.dark .frc-progress::-webkit-progress-bar{background:#444}.dark .frc-progress::-webkit-progress-value{background:#ddd}.dark .frc-progress::-moz-progress-bar{background:#ddd}@keyframes frc-fade-in{from{opacity:0}to{opacity:1}}';

// This is not an enum to save some bytes in the output bundle.
const SOLVER_TYPE_JS = 1;
const CHALLENGE_SIZE_BYTES = 128;

// @ts-ignore
const loaderSVG = `<circle cx="12" cy="12" r="8" stroke-width="3" stroke-dasharray="15 10" fill="none" stroke-linecap="round" transform="rotate(0 12 12)"><animateTransform attributeName="transform" type="rotate" repeatCount="indefinite" dur="0.9s" values="0 12 12;360 12 12"/></circle>`;
const errorSVG = `<path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-2h2v2zm0-4h-2V7h2v6z"/>`;
/**
 * Base template used for all widget states
 * The reason we use raw string interpolation here is so we don't have to ship something like lit-html.
 */
function getTemplate(fieldName, rtl, svgContent, svgAriaHidden, textContent, solutionString, buttonText, progress = false, debugData, additionalContainerClasses) {
    return `<div class="frc-container${additionalContainerClasses ? " " + additionalContainerClasses : ""}${rtl ? " frc-rtl" : ""}">
<svg class="frc-icon"${svgAriaHidden ? ' aria-hidden="true"' : ""} role="img" xmlns="http://www.w3.org/2000/svg" height="32" width="32" viewBox="0 0 24 24">${svgContent}</svg>
<div class="frc-content">
    <span class="frc-text" ${debugData ? `data-debug="${debugData}"` : ``}>${textContent}</span>
    ${buttonText ? `<button type="button" class="frc-button">${buttonText}</button>` : ""}
    ${progress ? `<progress class="frc-progress" value="0">0%</progress>` : ""}
</div>
</div><span class="frc-banner${rtl ? " frc-rtl" : ""}"><a lang="en" href="https://friendlycaptcha.com/" rel="noopener" target="_blank"><b>Friendly</b>Captcha ⇗</a></span>
${fieldName === "-" ? "" : `<input name="${fieldName}" class="frc-captcha-solution" type="hidden" value="${solutionString}">`}`;
}
/**
 * Used when the widget is ready to start solving.
 */
function getReadyHTML(fieldName, l) {
    return getTemplate(fieldName, l.rtl, `<path d="M17,11c0.34,0,0.67,0.04,1,0.09V6.27L10.5,3L3,6.27v4.91c0,4.54,3.2,8.79,7.5,9.82c0.55-0.13,1.08-0.32,1.6-0.55 C11.41,19.47,11,18.28,11,17C11,13.69,13.69,11,17,11z"/><path d="M17,13c-2.21,0-4,1.79-4,4c0,2.21,1.79,4,4,4s4-1.79,4-4C21,14.79,19.21,13,17,13z M17,14.38"/>`, true, l.text_ready, ".UNSTARTED", l.button_start, false);
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
    const timeData = `${data.t.toFixed(0)}s (${((data.h / data.t) * 0.001).toFixed(0)}K/s)${data.solver === SOLVER_TYPE_JS ? " JS Fallback" : ""}`;
    return getTemplate(fieldName, l.rtl, `<title>${l.text_completed_sr}</title><path d="M12 1L3 5v6c0 5.55 3.84 10.74 9 12 5.16-1.26 9-6.45 9-12V5l-9-4zm-2 16l-4-4 1.41-1.41L10 14.17l6.59-6.59L18 9l-8 8z"></path>`, false, l.text_completed, solution, undefined, false, timeData, "frc-success");
}
function getExpiredHTML(fieldName, l) {
    return getTemplate(fieldName, l.rtl, errorSVG, true, l.text_expired, ".EXPIRED", l.button_restart);
}
function getErrorHTML(fieldName, l, errorDescription, recoverable = true, headless = false) {
    return getTemplate(fieldName, l.rtl, errorSVG, true, `<b>${l.text_error}</b><br>${errorDescription}`, headless ? ".HEADLESS_ERROR" : ".ERROR", recoverable ? l.button_retry : undefined);
}
function findCaptchaElements() {
    const elements = document.querySelectorAll(".frc-captcha");
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
    if (!document.querySelector("#frc-style")) {
        const styleSheet = document.createElement("style");
        styleSheet.id = "frc-style";
        styleSheet.innerHTML = css;
        document.head.appendChild(styleSheet);
    }
}
/**
 * @param element parent element of friendlycaptcha
 * @param progress value between 0 and 1
 */
function updateProgressBar(element, data) {
    const p = element.querySelector(".frc-progress");
    const perc = (data.i + 1) / data.n;
    if (p) {
        p.value = perc;
        p.innerText = perc.toFixed(2) + "%";
        p.title = data.i + 1 + "/" + data.n + " (" + ((data.h / data.t) * 0.001).toFixed(0) + "K/s)";
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
    element.addEventListener("focusin", listener, { once: true, passive: true });
}

// Adapted from the base64-arraybuffer package implementation
// (https://github.com/niklasvh/base64-arraybuffer, MIT licensed)
const CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const EQ_CHAR = "=".charCodeAt(0);
// Use a lookup table to find the index.
const lookup = new Uint8Array(256);
for (let i = 0; i < CHARS.length; i++) {
    lookup[CHARS.charCodeAt(i)] = i;
}
function encode(bytes) {
    const len = bytes.length;
    let base64 = "";
    for (let i = 0; i < len; i += 3) {
        const b0 = bytes[i + 0];
        const b1 = bytes[i + 1];
        const b2 = bytes[i + 2];
        // This temporary variable stops the NextJS 13 compiler from breaking this code in optimization.
        // See issue https://github.com/FriendlyCaptcha/friendly-challenge/issues/165
        let t = "";
        t += CHARS.charAt(b0 >>> 2);
        t += CHARS.charAt(((b0 & 3) << 4) | (b1 >>> 4));
        t += CHARS.charAt(((b1 & 15) << 2) | (b2 >>> 6));
        t += CHARS.charAt(b2 & 63);
        base64 += t;
    }
    if (len % 3 === 2) {
        base64 = base64.substring(0, base64.length - 1) + "=";
    }
    else if (len % 3 === 1) {
        base64 = base64.substring(0, base64.length - 2) + "==";
    }
    return base64;
}
function decode(base64) {
    const len = base64.length;
    let bufferLength = (len * 3) >>> 2; // * 0.75
    if (base64.charCodeAt(len - 1) === EQ_CHAR)
        bufferLength--;
    if (base64.charCodeAt(len - 2) === EQ_CHAR)
        bufferLength--;
    const bytes = new Uint8Array(bufferLength);
    for (let i = 0, p = 0; i < len; i += 4) {
        const encoded1 = lookup[base64.charCodeAt(i + 0)];
        const encoded2 = lookup[base64.charCodeAt(i + 1)];
        const encoded3 = lookup[base64.charCodeAt(i + 2)];
        const encoded4 = lookup[base64.charCodeAt(i + 3)];
        bytes[p++] = (encoded1 << 2) | (encoded2 >> 4);
        bytes[p++] = ((encoded2 & 15) << 4) | (encoded3 >> 2);
        bytes[p++] = ((encoded3 & 3) << 6) | (encoded4 & 63);
    }
    return bytes;
}

// Defensive init to make it easier to integrate with Gatsby, NextJS, and friends.
let nav;
let ua;
if (typeof navigator !== "undefined") {
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
    ua.indexOf("headless") !== -1 ||
        nav.appVersion.indexOf("Headless") !== -1 ||
        ua.indexOf("bot") !== -1 || // http://www.useragentstring.com/pages/useragentstring.php?typ=Browser
        ua.indexOf("crawl") !== -1 || // Only IE5 has two distributions that has this on windows NT.. so yeah.
        nav.webdriver === true ||
        !nav.language ||
        (nav.languages !== undefined && !nav.languages.length) // IE 11 does not support NavigatorLanguage.languages https://developer.mozilla.org/en-US/docs/Web/API/NavigatorLanguage/languages
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
    }
    else if (value < 0) {
        value = 0;
    }
    return Math.pow(2, (255.999 - value) / 8.0) >>> 0;
}

const PUZZLE_EXPIRY_OFFSET = 13;
const NUMBER_OF_PUZZLES_OFFSET = 14;
const PUZZLE_DIFFICULTY_OFFSET = 15;
function getPuzzleSolverInputs(puzzleBuffer, numPuzzles) {
    const startingPoints = [];
    for (let i = 0; i < numPuzzles; i++) {
        const input = new Uint8Array(CHALLENGE_SIZE_BYTES);
        input.set(puzzleBuffer);
        input[120] = i;
        startingPoints.push(input);
    }
    return startingPoints;
}

function decodeBase64Puzzle(base64Puzzle) {
    const parts = base64Puzzle.split(".");
    const puzzle = parts[1];
    const arr = decode(puzzle);
    return {
        signature: parts[0],
        base64: puzzle,
        buffer: arr,
        n: arr[NUMBER_OF_PUZZLES_OFFSET],
        threshold: difficultyToThreshold(arr[PUZZLE_DIFFICULTY_OFFSET]),
        expiry: arr[PUZZLE_EXPIRY_OFFSET] * 300000,
    };
}
async function getPuzzle(urlsSeparatedByComma, siteKey, lang) {
    const urls = urlsSeparatedByComma.split(",");
    for (let i = 0; i < urls.length; i++) {
        try {
            const response = await fetchAndRetryWithBackoff(urls[i] + "?sitekey=" + siteKey, { headers: [["x-frc-client", "js-0.9.14"]], mode: "cors" }, 2);
            if (response.ok) {
                const json = await response.json();
                return json.data.puzzle;
            }
            else {
                let json;
                try {
                    json = await response.json();
                }
                catch (e) {
                    /* Do nothing, the error is not valid JSON */
                }
                if (json && json.errors && json.errors[0] === "endpoint_not_enabled") {
                    throw Error(`Endpoint not allowed (${response.status})`);
                }
                if (i === urls.length - 1) {
                    throw Error(`Response status ${response.status} ${response.statusText} ${json ? json.errors : ""}`);
                }
            }
        }
        catch (e) {
            console.error("[FRC Fetch]:", e);
            const err = new Error(`${lang.text_fetch_error} <a class="frc-err-url" href="${urls[i]}">${urls[i]}</a>`);
            err.rawError = e;
            throw err;
        }
    }
    // This code should never be reached.
    throw Error(`Internal error`);
}
/**
 * Retries given request with exponential backoff (starting with 1000ms delay, multiplying by 4 every time)
 * @param url Request (can be string url) to fetch
 * @param opts Options for fetch
 * @param n Number of times to attempt before giving up.
 */
async function fetchAndRetryWithBackoff(url, opts, n) {
    let time = 1000;
    return fetch(url, opts).catch(async (error) => {
        if (n === 0)
            throw error;
        await new Promise((r) => setTimeout(r, time));
        time *= 4;
        return fetchAndRetryWithBackoff(url, opts, n - 1);
    });
}

// English
const LANG_EN = {
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
    text_fetch_error: "Failed to connect to",
};
// French
const LANG_FR = {
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
    text_fetch_error: "Problème de connexion avec",
};
// German
const LANG_DE = {
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
    text_fetch_error: "Verbindungsproblem mit",
};
// Dutch
const LANG_NL = {
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
    text_fetch_error: "Verbinding mislukt met",
};
// Italian
const LANG_IT = {
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
    text_fetch_error: "Problema di connessione con",
};
// Portuguese
const LANG_PT = {
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
    text_fetch_error: "Falha de conexão com",
};
// Spanish
const LANG_ES = {
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
    text_fetch_error: "Error al conectarse a",
};
// Catalan
const LANG_CA = {
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
    text_fetch_error: "Error connectant a",
};
// Japanese
const LANG_JA = {
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
    text_fetch_error: "接続ができませんでした",
};
// Danish
const LANG_DA = {
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
    text_fetch_error: "Forbindelsen mislykkedes",
};
// Russian
const LANG_RU = {
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
    text_fetch_error: "Ошибка подключения",
};
// Swedish
const LANG_SV = {
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
    text_fetch_error: "Verifiering misslyckades",
};
// Turkish
const LANG_TR = {
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
    text_fetch_error: "Bağlantı başarısız oldu",
};
// Greek
const LANG_EL = {
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
    text_fetch_error: " Αποτυχία σύνδεσης με",
};
// Ukrainian
const LANG_UK = {
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
    text_fetch_error: "Не вдалось з’єднатись",
};
// Bulgarian
const LANG_BG = {
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
    text_fetch_error: "Неуспешно свързване с",
};
// Czech
const LANG_CS = {
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
    text_fetch_error: "Připojení se nezdařilo",
};
// Slovak
const LANG_SK = {
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
    text_fetch_error: "Pripojenie sa nepodarilo",
};
// Norwegian
const LANG_NO = {
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
    text_fetch_error: "Tilkoblingen mislyktes",
};
// Finnish
const LANG_FI = {
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
    text_fetch_error: "Yhteys epäonnistui",
};
// Latvian
const LANG_LV = {
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
    text_fetch_error: "Neizdevās izveidot savienojumu ar",
};
// Lithuanian
const LANG_LT = {
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
    text_fetch_error: "Nepavyko prisijungti prie",
};
// Polish
const LANG_PL = {
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
    text_fetch_error: "Nie udało się połączyć z",
};
// Estonian
const LANG_ET = {
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
    text_fetch_error: "Ühenduse loomine nurjus",
};
// Croatian
const LANG_HR = {
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
    text_fetch_error: "Nije moguće uspostaviti vezu",
};
// Serbian
const LANG_SR = {
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
    text_fetch_error: "Neuspelo povezivanje sa...",
};
// Slovenian
const LANG_SL = {
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
    text_fetch_error: "Povezave ni bilo mogoče vzpostaviti",
};
// Hungarian
const LANG_HU = {
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
    text_fetch_error: "Nem sikerült csatlakozni",
};
// Romanian
const LANG_RO = {
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
    text_fetch_error: "Nu s-a putut conecta",
};
// Chinese
const LANG_ZH = {
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
    text_fetch_error: "无法连接到",
};
// Traditional Chinese
const LANG_ZH_TW = {
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
    text_fetch_error: "無法連線到",
};
// Vietnamese
const LANG_VI = {
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
    text_fetch_error: "Không kết nối được",
};
// Hebrew
const LANG_HE = {
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
    rtl: true,
};
// Thai
const LANG_TH = {
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
const localizations = {
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
    // alternative language codes
    nb: LANG_NO,
};

function createDiagnosticsBuffer(solverID, timeToSolved) {
    const arr = new Uint8Array(3);
    const view = new DataView(arr.buffer);
    view.setUint8(0, solverID);
    view.setUint16(1, timeToSolved);
    return arr;
}

var workerString = "!function(){\"use strict\";const A=\"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/\",I=\"=\".charCodeAt(0),g=new Uint8Array(256);for(let I=0;I<A.length;I++)g[A.charCodeAt(I)]=I;function C(A){const I={},g=A.exports,C=g.memory,Q=g.__alloc,t=g.__retain,B=g.__rtti_base||-1;return I.__allocArray=(A,I)=>{const g=function(A){return new Uint32Array(C.buffer)[(B+4>>>2)+2*A]}(A),e=31-Math.clz32(g>>>6&31),o=I.length,i=Q(o<<e,0),r=Q(12,A),n=new Uint32Array(C.buffer);n[r+0>>>2]=t(i),n[r+4>>>2]=i,n[r+8>>>2]=o<<e;const E=C.buffer,s=new Uint8Array(E);if(16384&g)for(let A=0;A<o;++A)s[(i>>>e)+A]=t(I[A]);else s.set(I,i>>>e);return r},I.__getUint8Array=A=>{const I=new Uint32Array(C.buffer),g=I[A+4>>>2];return new Uint8Array(C.buffer,g,I[g-4>>>2]>>>0)},function(A,I={}){const g=A.__argumentsLength?I=>{A.__argumentsLength.value=I}:A.__setArgumentsLength||A.__setargc||(()=>({}));for(const C in A){if(!Object.prototype.hasOwnProperty.call(A,C))continue;const Q=A[C],t=C.split(\".\")[0];\"function\"==typeof Q&&Q!==g?(I[t]=(...A)=>(g(A.length),Q(...A))).original=Q:I[t]=Q}return I}(g,I)}class Q{constructor(A){this.b=new Uint8Array(128),this.h=new Uint32Array(16),this.t=0,this.c=0,this.v=new Uint32Array(32),this.m=new Uint32Array(32),this.outlen=A}}function t(A,I){return A[I]^A[I+1]<<8^A[I+2]<<16^A[I+3]<<24}function B(A,I,g,C,Q,t,B,e){const o=I[B],i=I[B+1],r=I[e],n=I[e+1];let E,s,w,c,a=A[g],D=A[g+1],h=A[C],f=A[C+1],y=A[Q],l=A[Q+1],u=A[t],N=A[t+1];E=a+h,s=(a&h|(a|h)&~E)>>>31,a=E,D=D+f+s,E=a+o,s=(a&o|(a|o)&~E)>>>31,a=E,D=D+i+s,w=u^a,c=N^D,u=c,N=w,E=y+u,s=(y&u|(y|u)&~E)>>>31,y=E,l=l+N+s,w=h^y,c=f^l,h=w>>>24^c<<8,f=c>>>24^w<<8,E=a+h,s=(a&h|(a|h)&~E)>>>31,a=E,D=D+f+s,E=a+r,s=(a&r|(a|r)&~E)>>>31,a=E,D=D+n+s,w=u^a,c=N^D,u=w>>>16^c<<16,N=c>>>16^w<<16,E=y+u,s=(y&u|(y|u)&~E)>>>31,y=E,l=l+N+s,w=h^y,c=f^l,h=c>>>31^w<<1,f=w>>>31^c<<1,A[g]=a,A[g+1]=D,A[C]=h,A[C+1]=f,A[Q]=y,A[Q+1]=l,A[t]=u,A[t+1]=N}const e=[4089235720,1779033703,2227873595,3144134277,4271175723,1013904242,1595750129,2773480762,2917565137,1359893119,725511199,2600822924,4215389547,528734635,327033209,1541459225],o=[0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,28,20,8,16,18,30,26,12,2,24,0,4,22,14,10,6,22,16,24,0,10,4,30,26,20,28,6,12,14,2,18,8,14,18,6,2,26,24,22,28,4,12,10,20,8,0,30,16,18,0,10,14,4,8,20,30,28,2,22,24,12,16,6,26,4,24,12,20,0,22,16,6,8,26,14,10,30,28,2,18,24,10,2,30,28,26,8,20,0,14,12,6,18,4,16,22,26,22,14,28,24,2,6,18,10,0,30,8,16,12,4,20,12,30,28,18,22,6,0,16,24,4,26,14,2,8,20,10,20,4,16,8,14,12,2,10,30,22,18,28,6,24,26,0,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,28,20,8,16,18,30,26,12,2,24,0,4,22,14,10,6];function i(A,I){const g=A.v,C=A.m;for(let I=0;I<16;I++)g[I]=A.h[I],g[I+16]=e[I];g[24]=g[24]^A.t,g[25]=g[25]^A.t/4294967296,I&&(g[28]=~g[28],g[29]=~g[29]);for(let I=0;I<32;I++)C[I]=t(A.b,4*I);for(let A=0;A<12;A++)B(g,C,0,8,16,24,o[16*A+0],o[16*A+1]),B(g,C,2,10,18,26,o[16*A+2],o[16*A+3]),B(g,C,4,12,20,28,o[16*A+4],o[16*A+5]),B(g,C,6,14,22,30,o[16*A+6],o[16*A+7]),B(g,C,0,10,20,30,o[16*A+8],o[16*A+9]),B(g,C,2,12,22,24,o[16*A+10],o[16*A+11]),B(g,C,4,14,16,26,o[16*A+12],o[16*A+13]),B(g,C,6,8,18,28,o[16*A+14],o[16*A+15]);for(let I=0;I<16;I++)A.h[I]=A.h[I]^g[I]^g[I+16]}function r(A,I){for(let I=0;I<16;I++)A.h[I]=e[I];A.b.set(I),A.h[0]^=16842752^A.outlen}async function n(){return(A,I,g=4294967295)=>{const C=function(A,I,g){if(128!=A.length)throw Error(\"Invalid input\");const C=A.buffer,t=new DataView(C),B=new Q(32);B.t=128;const e=t.getUint32(124,!0),o=e+g;for(let g=e;g<o;g++)if(t.setUint32(124,g,!0),r(B,A),i(B,!0),B.h[0]<I)return 0==ASC_TARGET?new Uint8Array(B.h.buffer):Uint8Array.wrap(B.h.buffer);return new Uint8Array(0)}(A,I,g);return[A,C]}}let E,s;Uint8Array.prototype.slice||Object.defineProperty(Uint8Array.prototype,\"slice\",{value:function(A,I){return new Uint8Array(Array.prototype.slice.call(this,A,I))}}),self.ASC_TARGET=0;const w=new Promise((A=>s=A));self.onerror=A=>{self.postMessage({type:\"error\",message:JSON.stringify(A)})},self.onmessage=async A=>{const Q=A.data;try{if(\"solver\"===Q.type){if(Q.forceJS){E=1;const A=await n();s(A)}else try{E=2;const A=WebAssembly.compile(function(A){const C=A.length;let Q=3*C>>>2;A.charCodeAt(C-1)===I&&Q--,A.charCodeAt(C-2)===I&&Q--;const t=new Uint8Array(Q);for(let I=0,Q=0;I<C;I+=4){const C=g[A.charCodeAt(I+0)],B=g[A.charCodeAt(I+1)],e=g[A.charCodeAt(I+2)],o=g[A.charCodeAt(I+3)];t[Q++]=C<<2|B>>4,t[Q++]=(15&B)<<4|e>>2,t[Q++]=(3&e)<<6|63&o}return t}(\"AGFzbQEAAAABKghgAABgAn9/AGADf39/AX9gAX8AYAR/f39/AGAAAX9gAX8Bf2ACf38BfwINAQNlbnYFYWJvcnQABAMMCwcGAwAAAQIFAQIABQMBAAEGFgR/AUEAC38BQQALfwBBAwt/AEHgDAsHbgkGbWVtb3J5AgAHX19hbGxvYwABCF9fcmV0YWluAAIJX19yZWxlYXNlAAMJX19jb2xsZWN0AAQHX19yZXNldAAFC19fcnR0aV9iYXNlAwMNVWludDhBcnJheV9JRAMCDHNvbHZlQmxha2UyYgAKCAELCvQSC5IBAQV/IABB8P///wNLBEAACyMBQRBqIgQgAEEPakFwcSICQRAgAkEQSxsiBmoiAj8AIgVBEHQiA0sEQCAFIAIgA2tB//8DakGAgHxxQRB2IgMgBSADShtAAEEASARAIANAAEEASARAAAsLCyACJAEgBEEQayICIAY2AgAgAkEBNgIEIAIgATYCCCACIAA2AgwgBAsEACAACwMAAQsDAAELBgAjACQBC7sCAQF/AkAgAUUNACAAQQA6AAAgACABakEEayICQQA6AAMgAUECTQ0AIABBADoAASAAQQA6AAIgAkEAOgACIAJBADoAASABQQZNDQAgAEEAOgADIAJBADoAACABQQhNDQAgAEEAIABrQQNxIgJqIgBBADYCACAAIAEgAmtBfHEiAmpBHGsiAUEANgIYIAJBCE0NACAAQQA2AgQgAEEANgIIIAFBADYCECABQQA2AhQgAkEYTQ0AIABBADYCDCAAQQA2AhAgAEEANgIUIABBADYCGCABQQA2AgAgAUEANgIEIAFBADYCCCABQQA2AgwgACAAQQRxQRhqIgFqIQAgAiABayEBA0AgAUEgTwRAIABCADcDACAAQgA3AwggAEIANwMQIABCADcDGCABQSBrIQEgAEEgaiEADAELCwsLcgACfyAARQRAQQxBAhABIQALIAALQQA2AgAgAEEANgIEIABBADYCCCABQfD///8DIAJ2SwRAQcAKQfAKQRJBORAAAAsgASACdCIBQQAQASICIAEQBiAAKAIAGiAAIAI2AgAgACACNgIEIAAgATYCCCAAC88BAQJ/QaABQQAQASIAQQxBAxABQYABQQAQBzYCACAAQQxBBBABQQhBAxAHNgIEIABCADcDCCAAQQA2AhAgAEIANwMYIABC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\")),Q=await async function(A){const I=await async function(A){const I={env:{abort(){throw Error(\"Wasm aborted\")}}};return{exports:C(await WebAssembly.instantiate(A,I))}}(A),g=I.exports.__retain(I.exports.__allocArray(I.exports.Uint8Array_ID,new Uint8Array(128)));let Q=I.exports.__getUint8Array(g);return(A,C,t=4294967295)=>{Q.set(A);const B=I.exports.solveBlake2b(g,C,t);Q=I.exports.__getUint8Array(g);const e=I.exports.__getUint8Array(B);return I.exports.__release(B),[Q,e]}}(await A);s(Q)}catch(A){console.log(\"FriendlyCaptcha failed to initialize WebAssembly, falling back to Javascript solver: \"+A.toString()),E=1;const I=await n();s(I)}self.postMessage({type:\"ready\",solver:E})}else if(\"start\"===Q.type){const A=await w;self.postMessage({type:\"started\"});let I,g=0;for(let C=0;C<256;C++){Q.puzzleSolverInput[123]=C;const[t,B]=A(Q.puzzleSolverInput,Q.threshold);if(0!==B.length){I=t;break}console.warn(\"FC: Internal error or no solution found\"),g+=Math.pow(2,32)-1}g+=new DataView(I.slice(-4).buffer).getUint32(0,!0),self.postMessage({type:\"done\",solution:I.slice(-8),h:g,puzzleIndex:Q.puzzleIndex,puzzleNumber:Q.puzzleNumber})}}catch(A){setTimeout((()=>{throw A}))}}}();";

// Defensive init to make it easier to integrate with Gatsby and friends.
let URL;
if (typeof window !== "undefined") {
    URL = window.URL || window.webkitURL;
}
class WorkerGroup {
    constructor() {
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
        this.readyCount = 0;
        this.startCount = 0;
        this.progressCallback = () => 0;
        this.readyCallback = () => 0;
        this.startedCallback = () => 0;
        this.doneCallback = () => 0;
        this.errorCallback = () => 0;
    }
    init() {
        this.terminateWorkers();
        this.progress = 0;
        this.totalHashes = 0;
        this.readyCount = 0;
        this.startCount = 0;
        // Setup four workers for now - later we could calculate this depending on the device
        this.workers = new Array(4);
        const workerBlob = new Blob([workerString], { type: "text/javascript" });
        for (let i = 0; i < this.workers.length; i++) {
            this.workers[i] = new Worker(URL.createObjectURL(workerBlob));
            this.workers[i].onerror = (e) => this.errorCallback(e);
            this.workers[i].onmessage = (e) => {
                const data = e.data;
                if (!data)
                    return;
                if (data.type === "ready") {
                    this.readyCount++;
                    this.solverType = data.solver;
                    // We are ready, when all workers are ready
                    if (this.readyCount == this.workers.length) {
                        this.readyCallback();
                    }
                }
                else if (data.type === "started") {
                    this.startCount++;
                    // We started, when the first worker starts working
                    if (this.startCount == 1) {
                        this.startTime = Date.now();
                        this.startedCallback();
                    }
                }
                else if (data.type === "done") {
                    if (data.puzzleNumber !== this.puzzleNumber)
                        return; // solution belongs to a previous puzzle
                    if (this.puzzleIndex < this.puzzleSolverInputs.length) {
                        this.workers[i].postMessage({
                            type: "start",
                            puzzleSolverInput: this.puzzleSolverInputs[this.puzzleIndex],
                            threshold: this.threshold,
                            puzzleIndex: this.puzzleIndex,
                            puzzleNumber: this.puzzleNumber,
                        });
                        this.puzzleIndex++;
                    }
                    this.progress++;
                    this.totalHashes += data.h;
                    this.progressCallback({
                        n: this.numPuzzles,
                        h: this.totalHashes,
                        t: (Date.now() - this.startTime) / 1000,
                        i: this.progress,
                    });
                    this.solutionBuffer.set(data.solution, data.puzzleIndex * 8);
                    // We are done, when all puzzles have been solved
                    if (this.progress == this.numPuzzles) {
                        const totalTime = (Date.now() - this.startTime) / 1000;
                        this.doneCallback({
                            solution: this.solutionBuffer,
                            h: this.totalHashes,
                            t: totalTime,
                            diagnostics: createDiagnosticsBuffer(this.solverType, totalTime),
                            solver: this.solverType,
                        });
                    }
                }
                else if (data.type === "error") {
                    this.errorCallback(data);
                }
            };
        }
    }
    setupSolver(forceJS = false) {
        const msg = { type: "solver", forceJS: forceJS };
        for (let i = 0; i < this.workers.length; i++) {
            this.workers[i].postMessage(msg);
        }
    }
    start(puzzle) {
        this.puzzleSolverInputs = getPuzzleSolverInputs(puzzle.buffer, puzzle.n);
        this.solutionBuffer = new Uint8Array(8 * puzzle.n);
        this.numPuzzles = puzzle.n;
        this.threshold = puzzle.threshold;
        this.puzzleIndex = 0;
        this.puzzleNumber++;
        for (let i = 0; i < this.workers.length; i++) {
            if (this.puzzleIndex === this.puzzleSolverInputs.length)
                break;
            this.workers[i].postMessage({
                type: "start",
                puzzleSolverInput: this.puzzleSolverInputs[i],
                threshold: this.threshold,
                puzzleIndex: this.puzzleIndex,
                puzzleNumber: this.puzzleNumber,
            });
            this.puzzleIndex++;
        }
    }
    terminateWorkers() {
        if (this.workers.length == 0)
            return;
        for (let i = 0; i < this.workers.length; i++) {
            this.workers[i].terminate();
        }
        this.workers = [];
    }
}

const PUZZLE_ENDPOINT_URL = "https://api.friendlycaptcha.com/api/v1/puzzle";
class WidgetInstance {
    constructor(element, options = {}) {
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
            startedCallback: () => 0,
            readyCallback: () => 0,
            doneCallback: () => 0,
            errorCallback: () => 0,
            sitekey: element.dataset["sitekey"] || "",
            language: element.dataset["lang"] || "en",
            solutionFieldName: element.dataset["solutionFieldName"] || "frc-captcha-solution",
        }, options);
        this.e = element;
        this.e.friendlyChallengeWidget = this;
        this.loadLanguage();
        // @ts-ignore Ignore is required as TS thinks that `this.lang` is not assigned yet, but it happens in `this.loadLanguage()` above.
        element.innerText = this.lang.text_init;
        if (!this.opts.skipStyleInjection) {
            injectStyle();
        }
        this.init(this.opts.startMode === "auto" || this.e.dataset["start"] === "auto");
    }
    init(forceStart) {
        if (this.hasBeenDestroyed) {
            console.error("FriendlyCaptcha widget has been destroyed using destroy(), it can not be used anymore.");
            return;
        }
        this.initWorkerGroup();
        if (forceStart) {
            this.start();
        }
        else if (this.e.dataset["start"] !== "none" &&
            (this.opts.startMode === "focus" || this.e.dataset["start"] === "focus")) {
            const form = findParentFormElement(this.e);
            if (form) {
                executeOnceOnFocusInEvent(form, () => this.start());
            }
            else {
                console.log("FriendlyCaptcha div seems not to be contained in a form, autostart will not work");
            }
        }
    }
    /**
     * Loads the configured language, or a language passed to this function.
     * Note that only the next update will be in the new language, consider calling `reset()` after switching languages.
     */
    loadLanguage(lang) {
        if (lang !== undefined) {
            this.opts.language = lang;
        }
        else if (this.e.dataset["lang"]) {
            this.opts.language = this.e.dataset["lang"];
        }
        if (typeof this.opts.language === "string") {
            let l = localizations[this.opts.language.toLowerCase()];
            if (l === undefined) {
                console.error('FriendlyCaptcha: language "' + this.opts.language + '" not found.');
                // Fall back to English
                l = localizations.en;
            }
            this.lang = l;
        }
        else {
            // We assign to a copy of the English language localization, so that any missing values will be English
            this.lang = Object.assign(Object.assign({}, localizations.en), this.opts.language);
        }
    }
    /**
     * Add a listener to the button that calls `this.start` on click.
     */
    makeButtonStart() {
        const b = this.e.querySelector("button");
        if (b) {
            b.addEventListener("click", (e) => this.start(), { once: true, passive: true });
            b.addEventListener("touchstart", (e) => this.start(), { once: true, passive: true });
        }
    }
    onWorkerError(e) {
        this.hasBeenStarted = false;
        this.needsReInit = true;
        if (this.expiryTimeout)
            clearTimeout(this.expiryTimeout);
        console.error("[FRC]", e);
        this.e.innerHTML = getErrorHTML(this.opts.solutionFieldName, this.lang, "Background worker error " + e.message);
        this.makeButtonStart();
        // Just out of precaution
        this.opts.forceJSFallback = true;
    }
    initWorkerGroup() {
        this.workerGroup.progressCallback = (progress) => {
            updateProgressBar(this.e, progress);
        };
        this.workerGroup.readyCallback = () => {
            this.e.innerHTML = getReadyHTML(this.opts.solutionFieldName, this.lang);
            this.makeButtonStart();
            this.opts.readyCallback();
        };
        this.workerGroup.startedCallback = () => {
            this.e.innerHTML = getRunningHTML(this.opts.solutionFieldName, this.lang);
            this.opts.startedCallback();
        };
        this.workerGroup.doneCallback = (data) => {
            const solutionPayload = this.handleDone(data);
            this.opts.doneCallback(solutionPayload);
            const callback = this.e.dataset["callback"];
            if (callback) {
                window[callback](solutionPayload);
            }
        };
        this.workerGroup.errorCallback = (e) => {
            this.onWorkerError(e);
        };
        this.workerGroup.init();
        this.workerGroup.setupSolver(this.opts.forceJSFallback);
    }
    expire() {
        this.hasBeenStarted = false;
        // Node.isConnected will be undefined in older browsers
        if (this.e.isConnected !== false) {
            this.e.innerHTML = getExpiredHTML(this.opts.solutionFieldName, this.lang);
            this.makeButtonStart();
        }
    }
    async start() {
        if (this.hasBeenDestroyed) {
            console.error("Can not start FriendlyCaptcha widget which has been destroyed");
            return;
        }
        if (this.hasBeenStarted) {
            console.warn("Can not start FriendlyCaptcha widget which has already been started");
            return;
        }
        const sitekey = this.opts.sitekey || this.e.dataset["sitekey"];
        if (!sitekey) {
            console.error("FriendlyCaptcha: sitekey not set on frc-captcha element");
            this.e.innerHTML = getErrorHTML(this.opts.solutionFieldName, this.lang, "Website problem: sitekey not set", false);
            return;
        }
        if (isHeadless()) {
            this.e.innerHTML = getErrorHTML(this.opts.solutionFieldName, this.lang, "Browser check failed, try a different browser", false, true);
            return;
        }
        if (this.needsReInit) {
            this.needsReInit = false;
            this.init(true);
            return;
        }
        this.hasBeenStarted = true;
        try {
            this.e.innerHTML = getFetchingHTML(this.opts.solutionFieldName, this.lang);
            this.puzzle = decodeBase64Puzzle(await getPuzzle(this.opts.puzzleEndpoint, sitekey, this.lang));
            if (this.expiryTimeout)
                clearTimeout(this.expiryTimeout);
            this.expiryTimeout = setTimeout(() => this.expire(), this.puzzle.expiry - 30000); // 30s grace
        }
        catch (e) {
            console.error("[FRC]", e);
            this.hasBeenStarted = false;
            if (this.expiryTimeout)
                clearTimeout(this.expiryTimeout);
            this.e.innerHTML = getErrorHTML(this.opts.solutionFieldName, this.lang, e.message);
            this.makeButtonStart();
            const code = "error_getting_puzzle";
            this.opts.errorCallback({ code, description: e.toString(), error: e });
            const callback = this.e.dataset["callback-error"];
            if (callback) {
                window[callback](this);
            }
            return;
        }
        this.workerGroup.start(this.puzzle);
    }
    /**
     * This is to be called when the puzzle has been succesfully completed.
     * Here the hidden field gets updated with the solution.
     * @param data message from the webworker
     */
    handleDone(data) {
        this.valid = true;
        const puzzleSolutionMessage = `${this.puzzle.signature}.${this.puzzle.base64}.${encode(data.solution)}.${encode(data.diagnostics)}`;
        this.e.innerHTML = getDoneHTML(this.opts.solutionFieldName, this.lang, puzzleSolutionMessage, data);
        // this.worker = null; // This literally crashes very old browsers..
        this.needsReInit = true;
        return puzzleSolutionMessage;
    }
    /**
     * Cleans up the widget entirely, removing any DOM elements and terminating any background workers.
     * After it is destroyed it can no longer be used for any purpose.
     */
    destroy() {
        this.workerGroup.terminateWorkers();
        this.needsReInit = false;
        this.hasBeenStarted = false;
        if (this.expiryTimeout)
            clearTimeout(this.expiryTimeout);
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
    reset() {
        if (this.hasBeenDestroyed) {
            console.error("FriendlyCaptcha widget has been destroyed, it can not be used anymore");
            return;
        }
        this.workerGroup.terminateWorkers();
        this.needsReInit = false;
        this.hasBeenStarted = false;
        if (this.expiryTimeout)
            clearTimeout(this.expiryTimeout);
        this.init(this.opts.startMode === "auto" || this.e.dataset["start"] === "auto");
    }
}

window.friendlyChallenge = {
    WidgetInstance: WidgetInstance,
};
function setup() {
    let autoWidget = window.friendlyChallenge.autoWidget;
    const elements = findCaptchaElements();
    for (let index = 0; index < elements.length; index++) {
        const hElement = elements[index];
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
}
else {
    document.addEventListener("DOMContentLoaded", setup);
}
