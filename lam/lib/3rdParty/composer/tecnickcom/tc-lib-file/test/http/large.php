<?php

/**
 * large.php - Test HTTP endpoint that returns a large body.
 *
 * Used by FileTest to verify that the maxRemoteSize limit is enforced
 * via the cURL progress callback.
 */

echo \str_repeat('X', 1000);
