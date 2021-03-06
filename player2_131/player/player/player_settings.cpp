/************************************************************************
COPYRIGHT (C) SGS-THOMSON Microelectronics 2006

Source file name : player_settings.cpp
Author :           Nick

Implementation of the settings related functions of the
generic class implementation of player 2


Date        Modification                                    Name
----        ------------                                    --------
03-Nov-06   Created                                         Nick

************************************************************************/

#include <stdarg.h>
#include "player_generic.h"

// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Useful defines/macros that need not be user visible
//

// Used for human readable policy reporting
static const char *LookupPlayerPolicy( PlayerPolicy_t Policy )
{
    switch( Policy )
    {
#define C(x) case x: return #x
    C(PolicyTrickModeAudio);
    C(PolicyPlay24FPSVideoAt25FPS);
    C(PolicyMasterClock);
    C(PolicyExternalTimeMapping);
    C(PolicyAVDSynchronization);
    C(PolicyManifestFirstFrameEarly);
    C(PolicyVideoBlankOnShutdown);
    C(PolicyStreamOnlyKeyFrames);
    C(PolicyStreamSingleGroupBetweenDiscontinuities);
    C(PolicyClampPlaybackIntervalOnPlaybackDirectionChange);
    C(PolicyPlayoutOnTerminate);
    C(PolicyPlayoutOnSwitch);
    C(PolicyPlayoutOnDrain);
    C(PolicyDisplayAspectRatio);
    C(PolicyDisplayFormat);
    C(PolicyTrickModeDomain);
    C(PolicyDiscardLateFrames);
    C(PolicyVideoStartImmediate);
    C(PolicyRebaseOnFailureToDeliverDataInTime);
    C(PolicyRebaseOnFailureToDecodeInTime);
    C(PolicyLowerCodecDecodeLimitsOnFailureToDecodeInTime);
    C(PolicyH264AllowNonIDRResynchronization);
    C(PolicyH264AllowBadPreProcessedFrames);
    C(PolicyMPEG2DoNotHonourProgressiveFrameFlag);
    C(PolicyClockPullingLimit2ToTheNPartsPerMillion);
    C(PolicyLimitInputInjectAhead);
    C(PolicyMPEG2ApplicationType);

    // Private policies (see player_generic.h)
    C(PolicyPlayoutAlwaysPlayout);
    C(PolicyPlayoutAlwaysDiscard);
    C(PolicyStatisticsOnAudio);
    C(PolicyStatisticsOnVideo);
#undef C

    default:
        if (Policy < (PlayerPolicy_t) PolicyMaxExtraPolicy)
	{
		report( severity_info, "LookupPlayerPolicy - Lookup failed for policy %d\n", Policy );
		return "UNKNOWN";
	}

	return "UNSPECIFIED";
    }
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Static constant data
//

static BufferDataDescriptor_t	  MetaDataInputDescriptor 			= METADATA_PLAYER_INPUT_DESCRIPTOR_TYPE;
static BufferDataDescriptor_t	  MetaDataCodedFrameParameters 			= METADATA_CODED_FRAME_PARAMETERS_TYPE;
static BufferDataDescriptor_t	  MetaDataStartCodeList 			= METADATA_START_CODE_LIST_TYPE;
static BufferDataDescriptor_t	  MetaDataParsedFrameParameters			= METADATA_PARSED_FRAME_PARAMETERS_TYPE;
static BufferDataDescriptor_t	  MetaDataParsedFrameParametersReference	= METADATA_PARSED_FRAME_PARAMETERS_REFERENCE_TYPE;
static BufferDataDescriptor_t	  MetaDataParsedVideoParameters			= METADATA_PARSED_VIDEO_PARAMETERS_TYPE;
static BufferDataDescriptor_t	  MetaDataParsedAudioParameters			= METADATA_PARSED_AUDIO_PARAMETERS_TYPE;
static BufferDataDescriptor_t	  MetaDataVideoOutputTiming			= METADATA_VIDEO_OUTPUT_TIMING_TYPE;
static BufferDataDescriptor_t	  MetaDataAudioOutputTiming			= METADATA_AUDIO_OUTPUT_TIMING_TYPE;
static BufferDataDescriptor_t	  MetaDataVideoOutputSurfaceDescriptor		= METADATA_VIDEO_OUTPUT_SURFACE_DESCRIPTOR_TYPE;
static BufferDataDescriptor_t	  MetaDataAudioOutputSurfaceDescriptor		= METADATA_AUDIO_OUTPUT_SURFACE_DESCRIPTOR_TYPE;
static BufferDataDescriptor_t	  MetaDataBufferStructure			= METADATA_BUFFER_STRUCTURE_TYPE;

static BufferDataDescriptor_t	  BufferVideoPostProcessingControl		= BUFFER_VIDEO_POST_PROCESSING_CONTROL_TYPE;
static BufferDataDescriptor_t	  BufferAudioPostProcessingControl		= BUFFER_AUDIO_POST_PROCESSING_CONTROL_TYPE;

static BufferDataDescriptor_t	  BufferPlayerControlStructure			= BUFFER_PLAYER_CONTROL_STRUCTURE_TYPE;
static BufferDataDescriptor_t	  BufferInputBuffer				= BUFFER_INPUT_TYPE;
static BufferDataDescriptor_t	  MetaDataSequenceNumberDescriptor		= METADATA_SEQUENCE_NUMBER_TYPE;

// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Register the buffer manager to the player 
//

PlayerStatus_t   Player_Generic_c::RegisterBufferManager(
						BufferManager_t		  BufferManager )
{
PlayerStatus_t		  Status;

//

    if( this->BufferManager != NULL )
    {
	report( severity_error, "Player_Generic_c::RegisterBufferManager - Attempt to change buffer manager, not permitted.\n" );
	return PlayerError;
    }

//

    this->BufferManager		= BufferManager;

    //
    // Register the predefined types, and the two buffer types used by the player
    //

    Status	= BufferManager->CreateBufferDataType( &MetaDataInputDescriptor, 		&MetaDataInputDescriptorType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataCodedFrameParameters, 		&MetaDataCodedFrameParametersType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataStartCodeList, 			&MetaDataStartCodeListType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataParsedFrameParameters,		&MetaDataParsedFrameParametersType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataParsedFrameParametersReference,	&MetaDataParsedFrameParametersReferenceType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataParsedVideoParameters,		&MetaDataParsedVideoParametersType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataParsedAudioParameters,		&MetaDataParsedAudioParametersType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataVideoOutputTiming,		&MetaDataVideoOutputTimingType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataAudioOutputTiming,		&MetaDataAudioOutputTimingType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataVideoOutputSurfaceDescriptor,	&MetaDataVideoOutputSurfaceDescriptorType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataAudioOutputSurfaceDescriptor,	&MetaDataAudioOutputSurfaceDescriptorType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataBufferStructure,		&MetaDataBufferStructureType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &MetaDataSequenceNumberDescriptor,	&MetaDataSequenceNumberType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &BufferVideoPostProcessingControl,	&BufferVideoPostProcessingControlType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &BufferAudioPostProcessingControl,	&BufferAudioPostProcessingControlType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &BufferPlayerControlStructure,		&BufferPlayerControlStructureType );
    if( Status != BufferNoError )
	return Status;

    Status	= BufferManager->CreateBufferDataType( &BufferInputBuffer,			&BufferInputBufferType );
    if( Status != BufferNoError )
	return Status;

    //
    // Create pools of buffers for input and player control structures
    //

    Status	= BufferManager->CreatePool( &PlayerControlStructurePool, BufferPlayerControlStructureType, PLAYER_MAX_CONTROL_STRUCTURE_BUFFERS );
    if( Status != BufferNoError )
    {
	report( severity_error, "Player_Generic_c::RegisterBufferManager - Failed to create pool of player control structures.\n" );
	return Status;
    }

//

    Status	= BufferManager->CreatePool( &InputBufferPool, BufferInputBufferType, PLAYER_MAX_INPUT_BUFFERS );
    if( Status != BufferNoError )
    {
	report( severity_error, "Player_Generic_c::RegisterBufferManager - Failed to create pool of input buffers.\n" );
	return Status;
    }

    Status	= InputBufferPool->AttachMetaData( MetaDataInputDescriptorType );
    if( Status != BufferNoError )
    {
	report( severity_error, "Player_Generic_c::RegisterBufferManager - Failed to attach an input descriptor to all input buffers.\n" );
	return Status;
    }

//

    return Status;
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Register a demultiplexor to the player (may be several registered at once)
//

PlayerStatus_t   Player_Generic_c::RegisterDemultiplexor(
						Demultiplexor_t		  Demultiplexor )
{
unsigned int		i;
PlayerInputMuxType_t	HandledType;
PlayerInputMuxType_t	PreviouslyHandledType;

//

    if( DemultiplexorCount >= PLAYER_MAX_DEMULTIPLEXORS )
    {
	report( severity_error, "Player_Generic_c::RegisterDemultiplexor - Too many demultiplexors.\n" );
	return PlayerError;
    }

//

    Demultiplexor->GetHandledMuxType( &HandledType );

    for( i=0; i<DemultiplexorCount; i++ )
    {
	Demultiplexor->GetHandledMuxType( &PreviouslyHandledType);
	if( HandledType == PreviouslyHandledType )
	{
	    report( severity_error, "Player_Generic_c::RegisterDemultiplexor - New demultiplexor duplicates previously registered one.\n" );
	    return PlayerError;
	}
    }

//

    Demultiplexors[DemultiplexorCount++]	= Demultiplexor;

    Demultiplexor->RegisterPlayer( this, PlayerAllPlaybacks, PlayerAllStreams );

//

    return PlayerNoError;
}

// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Set a policy
//
PlayerStatus_t   Player_Generic_c::SetPolicy(	PlayerPlayback_t	  Playback,
						PlayerStream_t		  Stream,
						PlayerPolicy_t		  Policy,
						unsigned char		  PolicyValue )
{
PlayerPolicyState_t	*SpecificPolicyRecord;

report( severity_info, "SetPolicy - %08x %08x %-45s %d\n", Playback, Stream, LookupPlayerPolicy(Policy), PolicyValue );
//

    if( (Playback		!= PlayerAllPlaybacks) &&
	(Stream			!= PlayerAllStreams) &&
	(Stream->Playback	!= Playback) )
    {
	report( severity_error, "Player_Generic_c::SetPolicy - Attempt to set policy on specific stream, and differing specific playback.\n" );
	return PlayerError;
    }

//

    if( Stream != PlayerAllStreams )
	SpecificPolicyRecord	= &Stream->PolicyRecord;
    else if( Playback != PlayerAllPlaybacks )
	SpecificPolicyRecord	= &Playback->PolicyRecord;
    else
	SpecificPolicyRecord	= &PolicyRecord;

//

    SpecificPolicyRecord->Specified[Policy/32]	|= 1 << (Policy % 32);
    SpecificPolicyRecord->Value[Policy]		 = PolicyValue;

//

    return PlayerNoError;
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Set module parameters
//

PlayerStatus_t   Player_Generic_c::SetModuleParameters(	
						PlayerPlayback_t	  Playback,
						PlayerStream_t		  Stream,
						PlayerComponent_t	  Component,
						bool			  Immediately,
						unsigned int		  ParameterBlockSize,
						void			 *ParameterBlock )
{
unsigned int		  i;
PlayerStatus_t		  Status;
PlayerStatus_t		  CurrentStatus;
PlayerPlayback_t	  SubPlayback;
PlayerStream_t		  SubStream;
PlayerSequenceType_t	  SequenceType;
PlayerComponentFunction_t Fn;

//

    Status	= PlayerNoError;

    //
    // Check we are given consistent parameters
    //

    if( (Playback		!= PlayerAllPlaybacks) &&
	(Stream			!= PlayerAllStreams) &&
	(Stream->Playback	!= Playback) )
    {
	report( severity_error, "Player_Generic_c::SetModuleParameters - Attempt to set parameters on specific stream, and differing specific playback.\n" );
	return PlayerError;
    }

//

    if( !Immediately &&
	((Component == ComponentPlayer) ||
	 (Component == ComponentDemultiplexor) ||
	 (Component == ComponentOutputCoordinator) ||
	 (Component == ComponentCollator)) )
    {
	report( severity_error, "Player_Generic_c::SetModuleParameters - Setting for Player, demultiplexor, collator, and output coordinator can only occur immediately.\n" );
	return PlayerError;
    }

//

    if( !Immediately &&
	(ParameterBlockSize >= PLAYER_MAX_INLINE_PARAMETER_BLOCK_SIZE) )
    {
	report( severity_error, "Player_Generic_c::SetModuleParameters - Parameter block size larger than soft limit (%d > %d).\n",
				ParameterBlockSize, PLAYER_MAX_INLINE_PARAMETER_BLOCK_SIZE );
	return PlayerError;
    }

//

    if( Stream != PlayerAllStreams )
	Playback	= Stream->Playback;

    //
    // Split out player and demultiplexor as not being stream or 
    // playback related, and only supported in immediate mode.
    //

    switch( Component )
    {
	case ComponentPlayer:
		report( severity_error, "Player_Generic_c::SetModuleParameters - Currently no module parameters supported for the player.\n" );
		return PlayerNotSupported;

	case ComponentDemultiplexor:
		for( i=0; i<DemultiplexorCount; i++ )
		{
		    CurrentStatus	= Demultiplexors[i]->SetModuleParameters(  ParameterBlockSize, ParameterBlock );
		    if( CurrentStatus != PlayerNoError )
			Status	= CurrentStatus;
		}
		return Status;

	default:
		break;		
    }

    //
    // Select the function to be called and when
    //

    switch( Component )
    {
	case ComponentFrameParser:	Fn	= FrameParserFnSetModuleParameters;	break;
	case ComponentCodec:		Fn	= CodecFnSetModuleParameters;		break;
	case ComponentOutputTimer:	Fn	= OutputTimerFnSetModuleParameters;	break;
	case ComponentManifestor:	Fn	= ManifestorFnSetModuleParameters;	break;
        default:			Fn	= 0xffffffff;				break;		
    }

    SequenceType	= Immediately ? SequenceTypeImmediate : SequenceTypeBeforeSequenceNumber;

    //
    // Perform two nested loops over affected playbacks and streams.
    // These should terminate appropriately for specific playbacks/streams
    //

    for( SubPlayback	 = ((Playback == PlayerAllPlaybacks) ? ListOfPlaybacks : Playback);
	 ((Playback == PlayerAllPlaybacks) ? (SubPlayback != NULL) : (SubPlayback == Playback));
	 SubPlayback	 = SubPlayback->Next )
    {
    	//
    	// Fudge the output timer to reference a particular stream and only call once per playback
    	//

	if( Component == ComponentOutputCoordinator )
	    Stream	= SubPlayback->ListOfStreams;

	for( SubStream	 = ((Stream == PlayerAllStreams) ? SubPlayback->ListOfStreams : Stream);
	     ((Stream == PlayerAllStreams) ? (SubStream != NULL) : (SubStream == Stream));
	     SubStream	 = SubStream->Next )
	{

	    if( Component == ComponentCollator )
		CurrentStatus	= SubStream->Collator->SetModuleParameters( ParameterBlockSize, ParameterBlock );
	    if( Component == ComponentOutputCoordinator )
		CurrentStatus	= SubPlayback->OutputCoordinator->SetModuleParameters( ParameterBlockSize, ParameterBlock );
	    else
		CurrentStatus	= CallInSequence( SubStream, SequenceType, SubStream->NextBufferSequenceNumber, Fn, ParameterBlockSize, ParameterBlock );

	    if( CurrentStatus != PlayerNoError )
		Status	= CurrentStatus;
	}
    }

//

    return Status;
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Set the presentation interval
//

PlayerStatus_t   Player_Generic_c::SetPresentationInterval(
						PlayerPlayback_t	  Playback,
						unsigned long long	  IntervalStartNativeTime,
						unsigned long long	  IntervalEndNativeTime )
{
PlayerStatus_t		Status;
unsigned long long	IntervalStartNormalizedTime;
unsigned long long	IntervalEndNormalizedTime;

    //
    // Translate the interval to normalized time
    //

    if( Playback->ListOfStreams == NULL )
    {
	report( severity_error, "Player_Generic_c::SetPresentationInterval - No streams to map native time to normalized time on.\n" );
	return PlayerUnknownStream;
    }

    Status	= Playback->ListOfStreams->FrameParser->TranslatePlaybackTimeNativeToNormalized( IntervalStartNativeTime, &IntervalStartNormalizedTime );
    if( Status == PlayerNoError )
	Status	= Playback->ListOfStreams->FrameParser->TranslatePlaybackTimeNativeToNormalized( IntervalEndNativeTime, &IntervalEndNormalizedTime );

    if( Status != PlayerNoError )
    {
	report( severity_error, "Player_Generic_c::SetPresentationInterval - Failed to translate native time to normalized time.\n" );
	return Status;
    }

    Playback->RequestedPresentationIntervalStartNormalizedTime	= IntervalStartNormalizedTime;
    Playback->RequestedPresentationIntervalEndNormalizedTime	= IntervalEndNormalizedTime;
    Playback->PresentationIntervalStartNormalizedTime		= IntervalStartNormalizedTime;
    Playback->PresentationIntervalEndNormalizedTime		= IntervalEndNormalizedTime;

    return PlayerNoError;
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Get the buffer manager
//

PlayerStatus_t   Player_Generic_c::GetBufferManager(
						BufferManager_t		 *BufferManager )
{
    *BufferManager	= this->BufferManager;

    if( this->BufferManager == NULL )
    {
	report( severity_error, "Player_Generic_c::GetBufferManager - BufferManager has not been registerred.\n" );
	return PlayerError;
    }

    return PlayerNoError;
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Find the class instances associated with a stream playback
//

PlayerStatus_t   Player_Generic_c::GetClassList(PlayerStream_t		  Stream,
						Collator_t		 *Collator,
						FrameParser_t		 *FrameParser,
						Codec_t			 *Codec,
						OutputTimer_t		 *OutputTimer,
						Manifestor_t		 *Manifestor )
{
    if( Collator != NULL )
	*Collator	= Stream->Collator;

    if( FrameParser != NULL )
	*FrameParser	= Stream->FrameParser;

    if( Codec != NULL )
	*Codec		= Stream->Codec;

    if( OutputTimer != NULL )
	*OutputTimer	= Stream->OutputTimer;

    if( Manifestor != NULL )
	*Manifestor	= Stream->Manifestor;

    return PlayerNoError;
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Find the coded frame buffer pool associated with a stream playback
//

PlayerStatus_t   Player_Generic_c::GetCodedFrameBufferPool(
						PlayerStream_t	 	  Stream,
						BufferPool_t		 *Pool,
						unsigned int		 *MaximumCodedFrameSize )
{
    *Pool			= Stream->CodedFrameBufferPool;

    if( MaximumCodedFrameSize != NULL )
	*MaximumCodedFrameSize	= Stream->MaximumCodedFrameSize;

    return PlayerNoError;
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Find the decode buffer pool associated with a stream playback
//

PlayerStatus_t   Player_Generic_c::GetDecodeBufferPool(
						PlayerStream_t	 	  Stream,
						BufferPool_t		 *Pool )
{
    *Pool	= Stream->DecodeBufferPool;
    return PlayerNoError;
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Find the post processing control buffer pool associated with a stream playback
//

PlayerStatus_t   Player_Generic_c::GetPostProcessControlBufferPool(
						PlayerStream_t	 	  Stream,
						BufferPool_t		 *Pool )
{
    *Pool	= Stream->PostProcessControlBufferPool;
    return PlayerNoError;
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Discover a playback speed
//

PlayerStatus_t   Player_Generic_c::GetPlaybackSpeed(
						PlayerPlayback_t	  Playback,
						Rational_t		 *Speed,
						PlayDirection_t		 *Direction )
{
    *Speed	= Playback->Speed;
    *Direction	= Playback->Direction;

    return PlayerNoError;
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Retrieve the presentation interval
//

PlayerStatus_t   Player_Generic_c::GetPresentationInterval(
						PlayerPlayback_t	  Playback,
						unsigned long long	 *IntervalStartNormalizedTime,
						unsigned long long	 *IntervalEndNormalizedTime )
{
    if( IntervalStartNormalizedTime != NULL )
	*IntervalStartNormalizedTime	= Playback->PresentationIntervalStartNormalizedTime;

    if( IntervalEndNormalizedTime != NULL )
	*IntervalEndNormalizedTime	= Playback->PresentationIntervalEndNormalizedTime;

    return PlayerNoError;
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
//
//      Discover the status of a policy
//

unsigned char	Player_Generic_c::PolicyValue(	PlayerPlayback_t	  Playback,
						PlayerStream_t		  Stream,
						PlayerPolicy_t		  Policy )
{
unsigned char		 Value;
unsigned int		 Offset;
unsigned int		 Mask;

//

    if( (Playback		!= PlayerAllPlaybacks) &&
	(Stream			!= PlayerAllStreams) &&
	(Stream->Playback	!= Playback) )
    {
	report( severity_error, "Player_Generic_c::PolicyValue - Attempt to get policy on specific stream, and differing specific playback.\n" );
	return PlayerError;
    }

//

    if( Stream != PlayerAllStreams )
	Playback	= Stream->Playback;

//

    Offset		= Policy/32;
    Mask		= (1 << (Policy % 32));

    Value		= 0;

    if( (PolicyRecord.Specified[Offset] & Mask) != 0 )
	Value	= PolicyRecord.Value[Policy];

    if( Playback != PlayerAllPlaybacks )
    {
	if( (Playback->PolicyRecord.Specified[Offset] & Mask) != 0 )
	    Value	= Playback->PolicyRecord.Value[Policy];

	if( Stream != PlayerAllStreams )
	{
	    if( (Stream->PolicyRecord.Specified[Offset] & Mask) != 0 )
		Value	= Stream->PolicyRecord.Value[Policy];
	}
    }

    return Value;
}


