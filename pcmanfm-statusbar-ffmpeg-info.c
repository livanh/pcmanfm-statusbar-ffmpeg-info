#include <stdlib.h>
#include <string.h>
#include <pcmanfm-modules.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

FM_DEFINE_MODULE( tab_page_status, ffmpeg-info )

static char *_sel_message( FmFileInfoList *files, gint n_files )
{
	FmFileInfo *fi;
	char* filename;
	AVFormatContext *fmt_ctx = NULL;
	AVCodecParameters *vparam, *aparam;
	int vstream_index, astream_index;
	int has_duration, has_video, has_audio;
	AVStream *vstream, *astream;
	char *duration_string, *video_string, *audio_string;
	char *description;
	int ret;
	
	if ( n_files > 1 ) {
		return NULL;
	}
	
	av_register_all();
	
	fi = fm_file_info_list_peek_head( files );
	
	filename = fm_path_to_str( fm_file_info_get_path( fi ) );
	ret = avformat_open_input( &fmt_ctx, filename, NULL, NULL );
	free( filename );
	if ( ret < 0 ) {
		return NULL;
	}
	
	ret = avformat_find_stream_info( fmt_ctx, NULL );
	if ( ret < 0 ) {
		return NULL;
	}
	
	if ( fmt_ctx->probe_score < 50 ) {
		return NULL;
	}
	
	if (
		strcmp( fmt_ctx->iformat->name, "image2" ) == 0 ||
		strcmp( fmt_ctx->iformat->name, "png_pipe" ) == 0 ||
		strcmp( fmt_ctx->iformat->name, "gif" ) == 0 ||
		strcmp( fmt_ctx->iformat->name, "tty" ) == 0
	) {
		return NULL;
	}
	
	vstream_index = av_find_best_stream( fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0 );
	if ( vstream_index >= 0 ) {
		has_video = 1;
	} else {
		has_video = 0;
	}
	
	astream_index = av_find_best_stream( fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0 );
	if ( astream_index >= 0 ) {
		has_audio = 1;
	} else {
		has_audio = 0;
	}
	
	if ( has_audio == 0 && has_video == 0 ) {
		return NULL;
	}
	
	if ( fmt_ctx->duration > 0 ) {
		int duration = fmt_ctx->duration / (float)AV_TIME_BASE;
		if ( duration > 3600 ) {
			duration_string = g_strdup_printf( "[%d:%02d:%02.2f]", duration/3600, (duration%3600)/60, duration%60 );
		} else {
			duration_string = g_strdup_printf( "[%d:%02d]", duration/60, duration%60 );
		}
		has_duration = 1;
	} else {
		has_duration = 0;
	}
	
	if ( has_video == 1 ) {
		
		char *fields_array[6];
		int n_fields = 0;
		vstream = fmt_ctx->streams[vstream_index];
		vparam = vstream->codecpar;
		
		const AVCodecDescriptor *vcodec = avcodec_descriptor_get( vparam->codec_id );
		
		if (
			(strcmp( vcodec->name, "mjpeg" ) == 0 || strcmp( vcodec->name, "png" ) == 0) &&
			(strcmp( fmt_ctx->iformat->name, "mp3" ) == 0 || strcmp( fmt_ctx->iformat->name, "ogg" ) == 0)
		) {
			has_video = 0;
			goto end_video;
		}
		
		fields_array[n_fields] = g_strdup_printf( "| %s", vcodec->name );
		n_fields++;
		
		if( vparam->bit_rate > 0 ) {
			fields_array[n_fields] = g_strdup_printf( "%d kbps", vparam->bit_rate/1000 );
			n_fields++;
		}
		
		if( vparam->width > 0 && vparam->height > 0 ) {
			fields_array[n_fields] = g_strdup_printf( "%dx%d", vparam->width, vparam->height );
			n_fields++;
		}
		
		if ( vstream->avg_frame_rate.num > 0 && vstream->avg_frame_rate.den > 0) {
			fields_array[n_fields] = g_strdup_printf( "%.2f fps", vstream->avg_frame_rate.num/(float)vstream->avg_frame_rate.den );
			n_fields++;
		}
		
		fields_array[n_fields] = NULL;
		video_string = g_strjoinv(
			", ",
			fields_array
		);
		
		int i;
		for( i=0; i<n_fields; i++ ) {
			free( fields_array[n_fields] );
		}
	}
	end_video:
	
	if ( has_audio == 1 ) {
		
		char *fields_array[6];
		int n_fields = 0;
		astream = fmt_ctx->streams[astream_index];
		aparam = astream->codecpar;
		
		const AVCodecDescriptor *acodec = avcodec_descriptor_get( aparam->codec_id );
		
		fields_array[n_fields] = g_strdup_printf( "| %s", acodec->name );
		n_fields++;
		
		if( aparam->bit_rate > 0) {
			fields_array[n_fields] = g_strdup_printf( "%d kbps", aparam->bit_rate/1000 );
			n_fields++;
		}
		
		if( aparam->sample_rate > 0) {
			fields_array[n_fields] = g_strdup_printf( "%d khz", aparam->sample_rate/1000 );
			n_fields++;
		}
		
		if( aparam->channels > 0 ) {
			char channels_description[20];
			av_get_channel_layout_string( channels_description, 20, aparam->channels, aparam->channel_layout );
			fields_array[n_fields] = g_strdup_printf( "%s", channels_description );
			n_fields++;
		}
		
		fields_array[n_fields] = NULL;
		audio_string = g_strjoinv(
			", ",
			fields_array
		);
		
		int i;
		for ( i=0; i<n_fields; i++ ) {
			free( fields_array[n_fields] );
		}
	}
	
	avformat_close_input( &fmt_ctx );
	
	if ( has_video == 1 || has_audio == 1 ) {
		
		char *fields_array[4];
		int n_fields = 0;
		
		if ( has_duration == 1 ) {
			fields_array[n_fields] = duration_string;
			n_fields++;
		}
		
		if ( has_video == 1 ) {
			fields_array[n_fields] = video_string;
			n_fields++;
		}
		
		if ( has_audio == 1 ) {
			fields_array[n_fields] = audio_string;
			n_fields++;
		}
		
		fields_array[n_fields] = NULL;
		description = g_strjoinv(
			" ",
			fields_array
		);
	}
	
	if ( has_duration == 1 ) {
		free( duration_string );
	}
	if ( has_video == 1 ) {
		free( video_string );
	}
	if ( has_audio == 1 ) {
		free( audio_string );
	}
	
	return description;
}

FmTabPageStatusInit fm_module_init_tab_page_status = {
	NULL,
	NULL,
	_sel_message
};
