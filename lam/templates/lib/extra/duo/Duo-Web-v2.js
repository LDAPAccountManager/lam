/**
 * Duo Web SDK v2
 * Copyright 2019, Duo Security
 */

(function (root, factory) {
    /*eslint-disable */
    if (typeof define === 'function' && define.amd) {
        // AMD. Register as an anonymous module.
        define([], factory);
    /*eslint-enable */
    } else if (typeof module === 'object' && module.exports) {
        // Node. Does not work with strict CommonJS, but
        // only CommonJS-like environments that support module.exports,
        // like Node.
        module.exports = factory();
    } else {
        // Browser globals (root is window)
        var Duo = factory();
        // If the Javascript was loaded via a script tag, attempt to autoload
        // the frame.
        Duo._onReady(Duo.init);

        // Attach Duo to the `window` object
        root.Duo = Duo;
  }
}(this, function() {
    var DUO_MESSAGE_FORMAT = /^(?:AUTH|ENROLL)+\|[A-Za-z0-9\+\/=]+\|[A-Za-z0-9\+\/=]+$/;
    var DUO_ERROR_FORMAT = /^ERR\|[\w\s\.\(\)]+$/;
    var DUO_OPEN_WINDOW_FORMAT = /^DUO_OPEN_WINDOW\|/;
    var VALID_OPEN_WINDOW_DOMAINS = [
        'duo.com',
        'duosecurity.com',
        'duomobile.s3-us-west-1.amazonaws.com'
    ];

    var postAction,
        postArgument,
        host,
        sigRequest,
        duoSig,
        appSig,
        iframe,
        submitCallback;

    // We use this function instead of setting initial values in the var
    // declarations to make sure the initial values and subsequent
    // re-initializations are always the same.
    initializeStatefulVariables();

    /**
     * Set local variables to whatever they should be before you call init().
     */
    function initializeStatefulVariables() {
        postAction = '';
        postArgument = 'sig_response';
        host = undefined;
        sigRequest = undefined;
        duoSig = undefined;
        appSig = undefined;
        iframe = undefined;
        submitCallback = undefined;
    }

    function throwError(message, givenUrl) {
        var url = (
            givenUrl ||
            'https://www.duosecurity.com/docs/duoweb#3.-show-the-iframe'
        );
        throw new Error(
            'Duo Web SDK error: ' + message +
            (url ? ('\n' + 'See ' + url + ' for more information') : '')
        );
    }

    function hyphenize(str) {
        return str.replace(/([a-z])([A-Z])/, '$1-$2').toLowerCase();
    }

    // cross-browser data attributes
    function getDataAttribute(element, name) {
        if ('dataset' in element) {
            return element.dataset[name];
        } else {
            return element.getAttribute('data-' + hyphenize(name));
        }
    }

    // cross-browser event binding/unbinding
    function on(context, event, fallbackEvent, callback) {
        if ('addEventListener' in window) {
            context.addEventListener(event, callback, false);
        } else {
            context.attachEvent(fallbackEvent, callback);
        }
    }

    function off(context, event, fallbackEvent, callback) {
        if ('removeEventListener' in window) {
            context.removeEventListener(event, callback, false);
        } else {
            context.detachEvent(fallbackEvent, callback);
        }
    }

    function onReady(callback) {
        on(document, 'DOMContentLoaded', 'onreadystatechange', callback);
    }

    function offReady(callback) {
        off(document, 'DOMContentLoaded', 'onreadystatechange', callback);
    }

    function onMessage(callback) {
        on(window, 'message', 'onmessage', callback);
    }

    function offMessage(callback) {
        off(window, 'message', 'onmessage', callback);
    }

    /**
     * Parse the sig_request parameter, throwing errors if the token contains
     * a server error or if the token is invalid.
     *
     * @param {String} sig Request token
     */
    function parseSigRequest(sig) {
        if (!sig) {
            // nothing to do
            return;
        }

        // see if the token contains an error, throwing it if it does
        if (sig.indexOf('ERR|') === 0) {
            throwError(sig.split('|')[1]);
        }

        // validate the token
        if (sig.indexOf(':') === -1 || sig.split(':').length !== 2) {
            throwError(
                'Duo was given a bad token.  This might indicate a configuration ' +
                'problem with one of Duo\'s client libraries.'
            );
        }

        var sigParts = sig.split(':');

        // hang on to the token, and the parsed duo and app sigs
        sigRequest = sig;
        duoSig = sigParts[0];
        appSig = sigParts[1];

        return {
            sigRequest: sig,
            duoSig: sigParts[0],
            appSig: sigParts[1]
        };
    }

    /**
     * Validate that a MessageEvent came from the Duo service, and that it
     * is a properly formatted payload.
     *
     * The Google Chrome sign-in page injects some JS into pages that also
     * make use of postMessage, so we need to do additional validation above
     * and beyond the origin.
     *
     * @param {MessageEvent} event Message received via postMessage
     */
    function isDuoMessage(event) {
        return Boolean(
            event.origin === ('https://' + host) &&
            typeof event.data === 'string' &&
            (
                event.data.match(DUO_MESSAGE_FORMAT) ||
                event.data.match(DUO_ERROR_FORMAT) ||
                event.data.match(DUO_OPEN_WINDOW_FORMAT)
            )
        );
    }

    /**
     * Validate the request token and prepare for the iframe to become ready.
     *
     * All options below can be passed into an options hash to `Duo.init`, or
     * specified on the iframe using `data-` attributes.
     *
     * Options specified using the options hash will take precedence over
     * `data-` attributes.
     *
     * Example using options hash:
     * ```javascript
     * Duo.init({
     *     iframe: "some_other_id",
     *     host: "api-main.duo.test",
     *     sig_request: "...",
     *     post_action: "/auth",
     *     post_argument: "resp"
     * });
     * ```
     *
     * Example using `data-` attributes:
     * ```html
     * <iframe id="duo_iframe"
     *         data-host="api-main.duo.test"
     *         data-sig-request="..."
     *         data-post-action="/auth"
     *         data-post-argument="resp"
     *         >
     * </iframe>
     * ```
     *
     * Some browsers (especially embedded browsers) don't like it when the Duo
     * Web SDK changes the `src` attribute on the iframe. To prevent this, there
     * is an alternative way to use the Duo Web SDK:
     *
     * Add a div (or any other container element) instead of an iframe to the
     * DOM with an id of "duo_iframe", or pass that element to the
     * `iframeContainer` parameter of `Duo.init`. An iframe will be created and
     * inserted into that container element, preventing `src` change related
     * bugs. WARNING: All other elements in the container will be deleted.
     *
     * The `iframeAttributes` parameter of `Duo.init` is available to set any
     * attributes on the inserted iframe if the Duo Web SDK is inserting the
     * iframe. For details, see the parameter documentation below.
     *
     * @param {Object} options
     * @param {String} options.host - Hostname for the Duo Prompt.
     * @param {String} options.sig_request - Request token.
     * @param {String|HTMLElement} [options.iframe] - The iframe, or id of an
     *     iframe that will be used for the Duo Prompt. If you don't provide
     *     this or the `iframeContainer` parameter the Duo Web SDK will default
     *     to using whatever element has an id of "duo_iframe".
     * @param {String|HTMLElement} [options.iframeContainer] - The element you
     *     want the Duo Prompt inserted into, or the id of that element.
     *     Anything inside this element will be deleted and replaced with an
     *     iframe hosting the Duo prompt. If you don't provide this or the
     *     `iframe` parameter the Duo Web SDK will default to using whatever
     *     element has an id of "duo_iframe".
     * @param {Object} [options.iframeAttributes] - Object with  names and
     *     values coresponding to attributes you want added to the  Duo Prompt
     *     iframe, like `title`, `width` and `allow`. WARNING: this parameter
     *     only works if you use the `iframeContainer` parameter or add an id
     *     of "duo_iframe" to an element that isn't an iframe. If you have
     *     added an iframe to the DOM yourself, you should set those attributes
     *     directly on the iframe.
     * @param {String} [options.post_action=''] - URL to POST back to after a
     *     successful auth.
     * @param {String} [options.post_argument='sig_response'] - Parameter name
     *     to use for response token.
     * @param {Function} [options.submit_callback] - If provided, the Duo Web
     *     SDK will not submit the form. Instead it will execute this callback
     *     function passing in a reference to the "duo_form" form object.
     *     `submit_callback`` can be used to prevent the webpage from reloading.
     */
    function init(options) {
        // If init() is called more than once we have to reset all the local
        // variables to ensure init() will work the same way every time. This
        // helps people making single page applications. SPAs may periodically
        // remove the iframe and add a new one that has to be initialized.
        initializeStatefulVariables();

        if (options) {
            if (options.host) {
                host = options.host;
            }

            if (options.sig_request) {
                parseSigRequest(options.sig_request);
            }

            if (options.post_action) {
                postAction = options.post_action;
            }

            if (options.post_argument) {
                postArgument = options.post_argument;
            }

            if (typeof options.submit_callback === 'function') {
                submitCallback = options.submit_callback;
            }
        }

        var promptElement = getPromptElement(options);
        if (promptElement) {
            // If we can get the element that will host the prompt, set it.
            ready(promptElement, options.iframeAttributes || {});
        } else {
            // If the element that will host the prompt isn't available yet, set
            // it up after the DOM finishes loading.
            asyncReady(options);
        }

        // always clean up after yourself!
        offReady(init);
    }

    /**
     * Given the options from init(), get the iframe or iframe container that
     * should be used for the Duo Prompt. Returns `null` if nothing was found.
     */
    function getPromptElement(options) {
        var result;

        if (options.iframe && options.iframeContainer) {
            throwError(
                'Passing both `iframe` and `iframeContainer` arguments at the' +
                ' same time is not allowed.'
            );
        } else if (options.iframe) {
            // If we are getting an iframe, try to get it and raise if the
            // element we find is NOT an iframe.
            result = getUserDefinedElement(options.iframe);
            validateIframe(result);
        } else if (options.iframeContainer) {
            result = getUserDefinedElement(options.iframeContainer);
            validateIframeContainer(result);
        } else {
            result = document.getElementById('duo_iframe');
        }

        return result;
    }

    /**
     * When given an HTMLElement, return it. When given a string, get an element
     * with that id, else return null.
     */
    function getUserDefinedElement(object) {
        if (object.tagName) {
            return object;
        } else if (typeof object == 'string') {
            return document.getElementById(object);
        }
        return null;
    }

    /**
     * Check if the given thing is an iframe.
     */
    function isIframe(element) {
        return (
            element &&
            element.tagName &&
            element.tagName.toLowerCase() === 'iframe'
        );
    }

    /**
     * Throw an error if we are given an element that is NOT an iframe.
     */
    function validateIframe(element) {
        if (element && !isIframe(element)) {
            throwError(
                '`iframe` only accepts an iframe element or the id of an' +
                ' iframe. To use a non-iframe element, use the' +
                ' `iframeContainer` argument.'
            );
        }
    }

    /**
     * Throw an error if we are given an element that IS an iframe instead of an
     * element that we can insert an iframe into.
     */
    function validateIframeContainer(element) {
        if (element && isIframe(element)) {
            throwError(
                '`iframeContainer` only accepts a non-iframe element or the' +
                ' id of a non-iframe. To use a non-iframe element, use the' +
                ' `iframeContainer` argument on Duo.init().'
            );
        }
    }

    /**
     * Generate the URL that goes to the Duo Prompt.
     */
    function generateIframeSrc() {
        return [
            'https://', host, '/frame/web/v1/auth?tx=', duoSig,
            '&parent=', encodeURIComponent(document.location.href),
            '&v=2.8'
        ].join('');
    }

    /**
     * This function is called when a message was received from another domain
     * using the `postMessage` API.  Check that the event came from the Duo
     * service domain, and that the message is a properly formatted payload,
     * then perform the post back to the primary service.
     *
     * @param event Event object (contains origin and data)
     */
    function onReceivedMessage(event) {
        if (isDuoMessage(event)) {
            if (event.data.match(DUO_OPEN_WINDOW_FORMAT)) {
                var url = event.data.substring("DUO_OPEN_WINDOW|".length);
                if (isValidUrlToOpen(url)) {
                    // Open the URL that comes after the DUO_WINDOW_OPEN token.
                    window.open(url, "_self");
                }
            }
            else {
                // the event came from duo, do the post back
                doPostBack(event.data);

                // always clean up after yourself!
                offMessage(onReceivedMessage);
            }
        }
    }

    /**
     * Validate that this passed in URL is one that we will actually allow to
     * be opened.
     * @param url String URL that the message poster wants to open
     * @returns {boolean} true if we allow this url to be opened in the window
     */
    function isValidUrlToOpen(url) {
        if (!url) {
            return false;
        }

        var parser = document.createElement('a');
        parser.href = url;

        if (parser.protocol === "duotrustedendpoints:") {
            return true;
        } else if (parser.protocol !== "https:") {
            return false;
        }

        for (var i = 0; i < VALID_OPEN_WINDOW_DOMAINS.length; i++) {
           if (parser.hostname.endsWith("." + VALID_OPEN_WINDOW_DOMAINS[i]) ||
                   parser.hostname === VALID_OPEN_WINDOW_DOMAINS[i]) {
               return true;
           }
        }
        return false;
    }

    /**
     * Register a callback to call ready() after the DOM has loaded.
     */
    function asyncReady(options) {
        var callback = function() {
            var promptElement = getPromptElement(options);
            if (!promptElement) {
                throwError(
                    'This page does not contain an iframe for Duo to use.' +
                    ' Add an element like' +
                    ' <iframe id="duo_iframe"></iframe> to this page.'
                );
            }

            ready(promptElement, options.iframeAttributes || {});

            // Always clean up after yourself.
            offReady(callback)
        };

        onReady(callback);
    }

    /**
     * Point the iframe at Duo, then wait for it to postMessage back to us.
     */
    function ready(promptElement, iframeAttributes) {
        if (!host) {
            host = getDataAttribute(promptElement, 'host');

            if (!host) {
                throwError(
                    'No API hostname is given for Duo to use.  Be sure to pass ' +
                    'a `host` parameter to Duo.init, or through the `data-host` ' +
                    'attribute on the iframe element.'
                );
            }
        }

        if (!duoSig || !appSig) {
            parseSigRequest(getDataAttribute(promptElement, 'sigRequest'));

            if (!duoSig || !appSig) {
                throwError(
                    'No valid signed request is given.  Be sure to give the ' +
                    '`sig_request` parameter to Duo.init, or use the ' +
                    '`data-sig-request` attribute on the iframe element.'
                );
            }
        }

        // if postAction/Argument are defaults, see if they are specified
        // as data attributes on the iframe
        if (postAction === '') {
            postAction = getDataAttribute(promptElement, 'postAction') || postAction;
        }

        if (postArgument === 'sig_response') {
            postArgument = getDataAttribute(promptElement, 'postArgument') || postArgument;
        }

        if (isIframe(promptElement)) {
            iframe = promptElement;
            iframe.src = generateIframeSrc();
        } else {
            // If given a container to put an iframe in, clean out any children
            // child elements in case `init()` was called more than once.
            while (promptElement.firstChild) {
                // We call `removeChild()` instead of doing `innerHTML = ""`
                // to make sure we unbind any events.
                promptElement.removeChild(promptElement.firstChild)
            }

            iframe = document.createElement('iframe');

            // Set the src and all other attributes on the new iframe.
            iframeAttributes['src'] = generateIframeSrc();
            for (var name in iframeAttributes) {
                iframe.setAttribute(name, iframeAttributes[name]);
            }

            promptElement.appendChild(iframe);
        }

        // listen for the 'message' event
        onMessage(onReceivedMessage);
    }

    /**
     * We received a postMessage from Duo.  POST back to the primary service
     * with the response token, and any additional user-supplied parameters
     * given in form#duo_form.
     */
    function doPostBack(response) {
        // create a hidden input to contain the response token
        var input = document.createElement('input');
        input.type = 'hidden';
        input.name = postArgument;
        input.value = response + ':' + appSig;

        // user may supply their own form with additional inputs
        var form = document.getElementById('duo_form');

        // if the form doesn't exist, create one
        if (!form) {
            form = document.createElement('form');

            // insert the new form after the iframe
            iframe.parentElement.insertBefore(form, iframe.nextSibling);
        }

        // make sure we are actually posting to the right place
        form.method = 'POST';
        form.action = postAction;

        // add the response token input to the form
        form.appendChild(input);

        // away we go!
        if (typeof submitCallback === "function") {
            submitCallback.call(null, form);
        } else {
            form.submit();
        }
    }

    return {
        init: init,
        _onReady: onReady,
        _parseSigRequest: parseSigRequest,
        _isDuoMessage: isDuoMessage,
        _doPostBack: doPostBack
    };
}));
