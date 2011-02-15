#ifndef __ENVIEW_ENVIREVISUALIZATION__
#define __ENVIEW_ENVIREVISUALIZATION__

#include <vizkit/VizPlugin.hpp>
#include <osg/Geometry>
#include <envire/Core.hpp>

#include <boost/thread/recursive_mutex.hpp>
#include "EnvireEventListener.hpp"
#include "TreeViewListener.hpp"

class QTreeWidget;

namespace vizkit 
{

class EnvireVisualization : public VizPluginAdapter<envire::Environment*>
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    EnvireVisualization();    

    void attachTreeWidget( QTreeWidget *treeWidget );
    bool isDirty() const;

    /** set to false if you want to manually handle the dirty flag by calling
     * setDirty() directly 
     */
    void handleDirty( bool handleDirty ) { m_handleDirty = handleDirty; }

protected:
    virtual void operatorIntern( osg::Node* node, osg::NodeVisitor* nv );
    virtual void updateDataIntern( envire::Environment* const& data );

private:
    bool m_handleDirty;

    envire::Environment *env;
    boost::recursive_mutex envLock;

    std::vector<boost::shared_ptr<EnvironmentItemVisualizer> > visualizers;

    boost::shared_ptr<EnvireEventListener> eventListener;
    boost::shared_ptr<TreeViewListener> twl;
};

}
#endif 
