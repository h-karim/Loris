<?php
/**
 * Implements the user password reset page for LORIS.
 *
 * PHP version 5
 *
 * @category   Behavioural
 * @package    Main
 * @subpackage Imaging
 * @author     Dave MacFarlane <david.macfarlane2@mcgill.ca>
 * @license    http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 * @link       https://www.github.com/aces/Loris/
 */
namespace LORIS\login;

/**
 * Implements the user password reset page for LORIS.
 *
 * @category   Behavioural
 * @package    Main
 * @subpackage Imaging
 * @author     Dave MacFarlane <david.macfarlane2@mcgill.ca>
 * @license    http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 * @link       https://www.github.com/aces/Loris/
 */
class PasswordReset extends \NDB_Form
{
    /**
     * CommonPageSetup sets up the common form elements which
     * are on the page, regardless of HTTP request method.
     *
     * @return void
     */
    function setup()
    {
        $this->tpl_data['page_title'] = 'Reset Password';
        $this->addBasicText("username", "", [], array("required" => true));

    }

    /**
     * _process updates the password of the user to a temporary password, if
     * username is a valid account. If not, it will still claim success for
     * security reasons.
     *
     * @param array $values The values that were submitted to the page.
     *
     * @return void
     */
    function _process($values)
    {
        // Always claim success to phishers.
        $this->tpl_data['success'] = 'Password reset. '
        . 'You should receive an email within a few minutes.';

        // create the user object
        $username = $values['username'];
        $user     = \User::factory($username);

        $email = $user->getData('Email');
        // check that it is a valid user
        if (empty($email) || empty($user)) {
            return;
        }

        try {
            // check that the email is valid
            if ($user->isEmailValid()) {
                $config = \NDB_Config::singleton();
                // generate a new password
                $password = \Utility::randomString();
                // reset the password in the database
                // expire password so user must change it upon login
                $user->updatePassword(
                    new \Password($password),
                    new \DateTime('1999-01-01')
                );

                // send the user an email
                $factory = \NDB_Factory::singleton();

                $msg_data          = array();
                $msg_data['study'] = $config->getSetting('title');
                $msg_data['url']   = $factory->settings()->getBaseURL();
                $msg_data['realname'] = $user->getData('Real_name');
                $msg_data['password'] = $password;
                \Email::send($email, 'lost_password.tpl', $msg_data);
            } else {
                error_log(
                    $_SERVER['REMOTE_ADDR']
                    . '(' . $_SERVER['HTTP_X_FORWARDED_FOR'] . ')'
                    . " attempted to reset password for $username"
                );
            }
        } catch(\Exception $e) {
            error_log(
                "Unhandled exception in reset_password: `{$e->getMessage()}`"
            );
        }
    }

}

