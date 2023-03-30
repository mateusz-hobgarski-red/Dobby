#include "UsbPlugin.h"
 
#include <iostream>
#include <sstream>
#include <thread>
#include <cstdio>

#include <sys/wait.h>
#include <sys/inotify.h>
#include <fcntl.h>

REGISTER_RDK_PLUGIN(UsbPlugin);

UsbPlugin::UsbPlugin(std::shared_ptr<rt_dobby_schema> &containerConfig,
                    const std::shared_ptr<DobbyRdkPluginUtils> &utils,
                    const std::string &rootfsPath)
    : mName("USB"),
      mRootfsPath(rootfsPath),
      mContainerConfig(containerConfig),
      mUtils(utils)
{
    AI_LOG_FN_ENTRY();
    AI_LOG_FN_EXIT();
}

UsbPlugin::~UsbPlugin()
{
    AI_LOG_FN_ENTRY();
    AI_LOG_FN_EXIT();
}

unsigned UsbPlugin::hookHints() const
{
    return (IDobbyRdkPlugin::HintFlags::PostInstallationFlag |
            IDobbyRdkPlugin::HintFlags::PostHaltFlag | 
            IDobbyRdkPlugin::HintFlags::PostStartFlag);
}

// Begin Hook Methods
bool UsbPlugin::postStart()
{
    AI_LOG_FN_ENTRY();
    AI_LOG_ERROR("MHMHMH postStart start");

    pid_t pid;
    AI_LOG_ERROR("MHMH Parent process starts with PID = %d it's Parent ID %d\n",(int)getpid(),(int)getppid());
    pid = fork();
    if (pid == -1)
    {
        AI_LOG_ERROR("FORK FAILED\n");
        return -1;
    }
    if (pid == 0 )
    {
        monitor();
    }
    else
    {
        AI_LOG_ERROR("Parent resumes execution, pid: %d, ppid: %d\n",(int)getpid(), (int)getppid());
    }

    AI_LOG_FN_EXIT();
    return true;
}
 
 /**
  * @brief Dobby Hook - run in host namespace *once* when container bundle is downloaded
  */
bool UsbPlugin::postInstallation()
{
    AI_LOG_FN_ENTRY();
    AI_LOG_ERROR("MHMHMH postInstallation start");

    addCapability("CAP_SYS_ADMIN");

    std::string path = mRootfsPath + "/run/media/sda1";
    if (!mUtils->mkdirRecursive(path, 0777) && errno != EEXIST)
    {
        AI_LOG_ERROR("failed to create directory '%s' (%d - %s)", path.c_str(), errno, strerror(errno));
        return false;
    }

    path = mRootfsPath + "/run/media/sdb1";
    if (!mUtils->mkdirRecursive(path, 0777) && errno != EEXIST)
    {
        AI_LOG_ERROR("failed to create directory '%s' (%d - %s)", path.c_str(), errno, strerror(errno));
        return false;
    }

    // following mkdirs fails with "pemission failed"
    // path = mRootfsPath + "/tmp/sda1";
    // if (!mUtils->mkdirRecursive(path, 0777) && errno != EEXIST)
    // {
    //     AI_LOG_ERROR("failed to create directory '%s' (%d - %s)", path.c_str(), errno, strerror(errno));
    //     return false;
    // }
    // path = mRootfsPath + "/tmp/sdb1";
    // if (!mUtils->mkdirRecursive(path, 0777) && errno != EEXIST)
    // {
    //     AI_LOG_ERROR("failed to create directory '%s' (%d - %s)", path.c_str(), errno, strerror(errno));
    //     return false;
    // }

    AI_LOG_FN_EXIT();
    return true;
}

bool UsbPlugin::postHalt()
{
    AI_LOG_FN_ENTRY();
    printf("MHMH postHalt start");


    AI_LOG_FN_EXIT();
    return true;    
}

// End hook methods

/**
 * @brief Should return the names of the plugins this plugin depends on.
 *
 * This can be used to determine the order in which the plugins should be
 * processed when running hooks.
 *
 * @return Names of the plugins this plugin depends on.
 */
std::vector<std::string> UsbPlugin::getDependencies() const
{
    std::vector<std::string> dependencies;
    const rt_defs_plugins_usb* pluginConfig = mContainerConfig->rdk_plugins->usb;

    for (size_t i = 0; i < pluginConfig->depends_on_len; i++)
    {
        dependencies.push_back(pluginConfig->depends_on[i]);
    }

    return dependencies;
}

bool UsbPlugin::monitor()
{
    log = fopen("/tmp/usb_plugin.log", "w");

    int fd = inotify_init();
    if (fd < 0)
    {
        fprintf(log, "Failed to init inotify\n");
        return false;
    }

    int wd = inotify_add_watch(fd, "/test", IN_CREATE);
    //wd = inotify_add_watch(fd, "/tmp/sda1", IN_CREATE);


    #define EVENT_SIZE  ( sizeof (struct inotify_event) )
    #define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
    char buffer[EVENT_BUF_LEN];
    int i = 0;
    int length = 0;


    fprintf(log, "Child process starts with pid: %d, ppid: %d, rootFsPath:%s\n", (int)getpid(), (int)getppid(), mRootfsPath.c_str());
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        length = read( fd, buffer, EVENT_BUF_LEN ); 
        while ( i < length )
        {
            struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
            if ( event->len )
            {
                if ( event->mask & IN_CREATE )
                {
                    fprintf(log, "New file %s created.\n", event->name );
                    fflush(log);

                    mUtils->callInNamespace(getpid(), CLONE_NEWNS, &UsbPlugin::doStuff, this, mRootfsPath + std::string("/tmp/") + std::string(event->name));

                    fprintf(log, "After callInNamespace.\n");
                    fflush(log);

                }
            }
            i += EVENT_SIZE + event->len;
        }

        i = length = 0;
    }

    inotify_rm_watch(fd, wd);
    close(fd);

    return true;
}

bool UsbPlugin::doStuff(const std::string& pathToCreate)
{
    std::string newName = pathToCreate + "1";
    fprintf(log, "doStuff path '%s'.\n", newName.c_str());

    // int fd = open(newName.c_str(), O_RDWR|O_CREAT, 0777);
    // if (fd < 0)
    // {
    //     fprintf(log, "open error: %s\n", strerror(errno));
    // }
    // fprintf(log, "doStuff path fd: %d.\n", fd);

    char cmd[256];
    sprintf(cmd, "nsenter -t %d -m -u -i -n -p -C -U mount --bind /tmp/sda1 /run/media/sda1", mUtils->getContainerPid());
    fprintf(log, "doStuff running command \"%s\".\n", cmd);

    system(cmd);
    // if (mount("/tmp/sda1", "/run/media/sda1", "", MS_BIND, nullptr) != 0)
    // {
    //     fprintf(log, "mount error: %s\n", strerror(errno));
    // }
    // else    
    //     fprintf(log, "mount ok!\n");

    fflush(log);
    // close(fd);

    return true;
}

bool UsbPlugin::addCapability(const std::string& cap)
{
    rt_dobby_schema_process_capabilities* capabilities = mContainerConfig->process->capabilities;

    capabilities->bounding_len++;
    capabilities->bounding = (char**)realloc(capabilities->bounding, sizeof(char*) * capabilities->bounding_len);
    capabilities->bounding[capabilities->bounding_len - 1] = strdup(cap.c_str());

    capabilities->permitted_len++;
    capabilities->permitted = (char**)realloc(capabilities->permitted, sizeof(char*) * capabilities->permitted_len);
    capabilities->permitted[capabilities->permitted_len - 1] = strdup(cap.c_str());

    capabilities->effective_len++;
    capabilities->effective = (char**)realloc(capabilities->effective, sizeof(char*) * capabilities->effective_len);
    capabilities->effective[capabilities->effective_len - 1] = strdup(cap.c_str());

    capabilities->inheritable_len++;
    capabilities->inheritable = (char**)realloc(capabilities->inheritable, sizeof(char*) * capabilities->inheritable_len);
    capabilities->inheritable[capabilities->inheritable_len - 1] = strdup(cap.c_str());

    capabilities->ambient_len++;
    capabilities->ambient = (char**)realloc(capabilities->ambient, sizeof(char*) * capabilities->ambient_len);
    capabilities->ambient[capabilities->ambient_len - 1] = strdup(cap.c_str());

    return true;
}
