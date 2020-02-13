<?php
/**
 * Copyright 2012-2017 Horde LLC (http://www.horde.org/)
 *
 * See the enclosed file COPYING for license information (LGPL). If you
 * did not receive this file, see http://www.horde.org/licenses/lgpl21.
 *
 * @author   Michael Slusarz <slusarz@horde.org>
 * @category Horde
 * @license  http://www.horde.org/licenses/lgpl21 LGPL 2.1
 * @package  ListHeaders
 */

/**
 * Class to parse List Header fields (RFC 2369/2919).
 *
 * @author   Michael Slusarz <slusarz@horde.org>
 * @category Horde
 * @license  http://www.horde.org/licenses/lgpl21 LGPL 2.1
 * @package  ListHeaders
 */
class Horde_ListHeaders extends Horde_Mail_Rfc822
{
    /**
     * Returns the list of valid mailing list headers.
     *
     * @return array  The list of valid mailing list headers.
     */
    public function headers()
    {
        return array(
            /* RFC 2369 */
            'list-help'         =>  Horde_ListHeaders_Translation::t("Help"),
            'list-unsubscribe'  =>  Horde_ListHeaders_Translation::t("Unsubscribe"),
            'list-subscribe'    =>  Horde_ListHeaders_Translation::t("Subscribe"),
            'list-owner'        =>  Horde_ListHeaders_Translation::t("Owner"),
            'list-post'         =>  Horde_ListHeaders_Translation::t("Post"),
            'list-archive'      =>  Horde_ListHeaders_Translation::t("Archive"),
            /* RFC 2919 */
            'list-id'           =>  Horde_ListHeaders_Translation::t("Identification")
        );
    }

   /**
    * Do any mailing list headers exist?
    *
    * @since 1.2.0
    *
    * @param Horde_Mime_Headers $ob  Headers object.
    *
    * @return boolean  True if any mailing list headers exist.
    */
    public function listHeadersExist(Horde_Mime_Headers $ob)
    {
        foreach (array_keys($this->headers()) as $hdr) {
            if (isset($ob[$hdr])) {
                return true;
            }
        }

        return false;
    }

    /**
     * Parse a list header.
     *
     * @param string $id     Header ID.
     * @param string $value  Header value.
     *
     * @return mixed  An array of Horde_ListHeaders_Base objects, a
     *                Horde_ListHeaders_Id object, or false if unable to
     *                parse.
     */
    public function parse($id, $value)
    {
        if (!strlen($value)) {
            return false;
        }

        $this->_data = strval($value);
        $this->_datalen = strlen($this->_data);
        $this->_params['validate'] = true;

        switch (Horde_String::lower($id)) {
        case 'list-archive':
        case 'list-help':
        case 'list-owner':
        case 'list-subscribe':
        case 'list-unsubscribe':
            return $this->_parseBase();

        case 'list-id':
            return $this->_parseListId();

        case 'list-post':
            return $this->_parseListPost();

        default:
            return false;
        }
    }

    /**
     * Parse a base list header (RFC 2369).
     *
     * @return array  List of Horde_List_Headers_Base objects.
     */
    protected function _parseBase()
    {
        $this->_ptr = 0;

        $out = array();

        while ($this->_curr() !== false) {
            $this->_comments = array();

            $this->_rfc822SkipLwsp();

            if ($this->_curr(true) != '<') {
                break;
            }

            $this->_rfc822SkipLwsp();

            $url = '';
            while ((($curr = $this->_curr(true)) !== false) &&
                   ($curr != '>')) {
                $url .= $curr;
            }

            if ($curr != '>') {
                return false;
            }

            $this->_rfc822SkipLwsp();

            switch ($this->_curr()) {
            case ',':
                $this->_rfc822SkipLwsp(true);
                break;

            case false:
                // No-op
                break;

            default:
                // RFC 2369 [2] Need to ignore this and all other fields.
                break 2;
            }

            $out[] = new Horde_ListHeaders_Base(rtrim($url), $this->_comments);
        }

        return $out;
    }

    /**
     * Parse a List-ID (RFC 2919).
     *
     * @return Horde_ListHeaders_Id  Id object.
     */
    protected function _parseListId()
    {
        $this->_ptr = 0;

        $phrase = '';
        $this->_rfc822ParsePhrase($phrase);

        if ($this->_curr(true) != '<') {
            return false;
        }

        $this->_rfc822ParseDotAtom($listid);

        if ($this->_curr(true) != '>') {
            return false;
        }

        return new Horde_ListHeaders_Id($listid, $phrase);
    }

    /**
     * Parse a List-Post header (RFC 2369 [3.4]).
     *
     * @return array  List of Horde_List_Headers_Base objects.
     */
    protected function _parseListPost()
    {
        /* This value can be the special phrase "NO". */
        $this->_comments = array();
        $this->_ptr = 0;

        $this->_rfc822SkipLwsp();

        $phrase = '';
        $this->_rfc822ParsePhrase($phrase);

        if (strcasecmp(rtrim($phrase), 'NO') !== 0) {
            return $this->_parseBase();
        }

        $this->_rfc822SkipLwsp();
        return array(new Horde_ListHeaders_NoPost($this->_comments));
    }

}
