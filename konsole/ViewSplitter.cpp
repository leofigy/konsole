/*
    This file is part of the Konsole Terminal.
    
    Copyright (C) 2006 Robert Knight <robertknight@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Konsole
#include "ViewContainer.h"
#include "ViewSplitter.h"

void ViewSplitter::childEmpty(ViewSplitter* splitter)
{
 //   qDebug() << __FUNCTION__ << ": deleting child splitter " ;

    delete splitter;

    if ( count() == 0 )
        emit empty(this);
}

ViewSplitter* ViewSplitter::activeSplitter()
{
   // qDebug() << "BEGIN activeSplitter" ;
    
    QWidget* widget = focusWidget() ? focusWidget() : this;
    
    ViewSplitter* splitter = 0;

    while ( !splitter && widget )
    {
       // qDebug() << widget ;
        splitter = dynamic_cast<ViewSplitter*>(widget);
        widget = widget->parentWidget();
    }

    Q_ASSERT( splitter );

    //qDebug() << "END activeSplitter";
    
    return splitter;
}

void ViewSplitter::registerContainer( ViewContainer* container )
{
    _containers << container;

    //qDebug() << __FUNCTION__ << ": adding container " << ((QTabWidget*)container->containerWidget())->tabText(0);
    
    connect( container , SIGNAL(destroyed(ViewContainer*)) , this , SLOT( containerDestroyed(ViewContainer*) ) );
}

void ViewSplitter::unregisterContainer( ViewContainer* container )
{
    _containers.removeAll(container);

    //qDebug() << __FUNCTION__ << ": removing container " << ((QTabWidget*)container->containerWidget())->tabText(0);
    
    disconnect( container , 0 , this , 0 );
}

void ViewSplitter::updateSizes()
{
    int space;

    if ( orientation() == Qt::Horizontal )
    {
        space = width() / count();
    }
    else
    {
        space = height() / count();
    }

    QList<int> widgetSizes;
    for (int i=0;i<count();i++)
        widgetSizes << space;

    setSizes(widgetSizes);
}

void ViewSplitter::addContainer( ViewContainer* container , 
                                 Qt::Orientation containerOrientation )
{
    
   ViewSplitter* splitter = activeSplitter();   
    
    if ( splitter->count() < 2 || containerOrientation == splitter->orientation() )
    {
        splitter->registerContainer(container); 
        splitter->addWidget(container->containerWidget());

        if ( splitter->orientation() != containerOrientation )
            splitter->setOrientation( containerOrientation );
        
        splitter->updateSizes();
    }
    else
    {
        ViewSplitter* newSplitter = new ViewSplitter();
        connect( newSplitter , SIGNAL(empty(ViewSplitter*)) , splitter , SLOT(childEmpty(ViewSplitter*)) );

        ViewContainer* oldContainer = splitter->activeContainer();
        int oldContainerIndex = splitter->indexOf(oldContainer->containerWidget());
     
        splitter->unregisterContainer(oldContainer);   
      
        newSplitter->registerContainer(oldContainer);
        newSplitter->registerContainer(container);
        
        newSplitter->addWidget(oldContainer->containerWidget());
        newSplitter->addWidget(container->containerWidget());
        newSplitter->setOrientation(containerOrientation); 
        newSplitter->updateSizes();
         
        splitter->insertWidget(oldContainerIndex,newSplitter);
    }
}

void ViewSplitter::containerDestroyed(ViewContainer* object)
{
    Q_ASSERT( _containers.contains(object) );
    
    _containers.removeAll(object);

    //qDebug() << __FUNCTION__ << ": remaining widgets = " << count();
    
    if ( count() == 0 )
    {
        emit empty(this);
    }
}

ViewContainer* ViewSplitter::activeContainer() const
{
   if ( QWidget* focusW = focusWidget() )
   {
      // qDebug() << __FUNCTION__ << ": focus-widget = " << focusW ;
      // if ( dynamic_cast<QLineEdit*>(focusW) )
       //    qDebug() << __FUNCTION__ << ": focus-widget-text = " << ((QLineEdit*)focusW)->text();
      // qDebug() << __FUNCTION__ << ": container count = " << _containers.count();

        ViewContainer* focusContainer = 0;
        
        while ( focusW != 0 )
        {
            QListIterator<ViewContainer*> containerIter(_containers);
            while (containerIter.hasNext())
            {
                ViewContainer* nextContainer = containerIter.next();
                             
                if (nextContainer->containerWidget() == focusW)
                {
                    focusContainer = nextContainer;
                    break;
                }
            }
            focusW = focusW->parentWidget();
        }

        if ( focusContainer )
            return focusContainer;
   }
    
   QList<ViewSplitter*> splitters = findChildren<ViewSplitter*>();

   if (splitters.count() > 0)
   {
        return splitters.last()->activeContainer();
   }
   else
   {
       if ( _containers.count() > 0 )
           return _containers.last();
       else
           return 0;
   }
}

#include "ViewSplitter.moc"
