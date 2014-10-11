/* vim: set ai ts=8 sw=4 sts=4 noet: */
#ifndef __VISUALIZER_H__
#define __VISUALIZER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define TYPE_VISUALIZER		(visualizer_get_type())
#define VISUALIZER(object)	(G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_VISUALIZER, Visualizer))
#define VISUALIZER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TYPE_VISUALIZER, VisualizerClass))
#define IS_VISUALIZER(object)	(G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_VISUALIZER))
#define IS_VISUALIZER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_VISUALIZER))
typedef struct _Visualizer Visualizer;
typedef struct _VisualizerClass VisualizerClass;

typedef enum _VisualizerStyle {
    VISUALIZER_OFF,
    VISUALIZER_ANALYZER,
    VISUALIZER_SCOPE,
} VisualizerStyle;

GType visualizer_get_type(void) G_GNUC_CONST;
Visualizer *visualizer_new(void);

inline void visualizer_set_visible(Visualizer * self, gboolean visible);
inline gboolean visualizer_get_visible(Visualizer * self);

void visualizer_set_style(Visualizer * self, VisualizerStyle style);
VisualizerStyle visualizer_get_style(Visualizer * self);

void visualizer_update(Visualizer * self);
G_END_DECLS
#endif /*__VISUALIZER_H__ */
