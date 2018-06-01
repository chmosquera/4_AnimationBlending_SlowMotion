#pragma once
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

class keyframe
	{
	public:
		quat quaternion;
		vec3 translation;
		long long timestamp_ms;
	};
class animation_per_bone
	{
	public:
		string name;
		long long duration;
		int frames;
		string bone;
		vector<keyframe> keyframes;
	};
class all_animations
	{
	public:
		vector<animation_per_bone> animations;
		vector<animation_per_bone> animations2;

	};


//**************************************************

class bone
{
public:
	vector<animation_per_bone*> animation; //all the animations of the bone
	string name;
	vec3 pos, pos2;
	quat q, q2;
	bone *parent = NULL;
	vector<bone*> kids;			
	unsigned int index;			//a unique number for each bone, at the same time index of the animatiom matrix array
	mat4 *mat = NULL;			//address of one element from the animation matrix array
	// searches for the animation and sets the animation matrix element to the recent matrix gained from the keyframe
	void play_animation(float keyframenumber, string animationname, float r) {
		float t = keyframenumber - (int)keyframenumber;
		//cout << keyframenumber << endl;

		for (int i = 0; i < animation.size(); i++)
			if (animation[i]->name == animationname)
				{
				int frame = (int)keyframenumber % animation[i]->keyframes.size();
				if (frame+1 < animation[i]->keyframes.size()) {
					//cout << "frame: " << frame << endl;
					quat qa = animation[i]->keyframes[frame].quaternion;
					quat qb = animation[i]->keyframes[frame+1].quaternion;
					vec3 ta = animation[i]->keyframes[frame].translation;
					vec3 tb = animation[i]->keyframes[frame+1].translation;
					quat qr = slerp(qa, qb, t);
					vec3 tr = mix(ta, tb, vec3(t));

					//after getting the translation like: vec3 tr = animation[animation_num]->keyframes[keyframenumber].translation;
					// stop translation - figure moves in one spot
					if (name == "Humanoid:Hips")
						tr = vec3(0, 0, 0);

					mat4 M = mat4(qr);
					mat4 T = translate(mat4(1), tr);
					M = T * M;
					if (mat)
						{
						mat4 parentmat = mat4(1);
						if (parent)
							parentmat = *parent->mat;
						*mat = parentmat * M;
						}
					}
				else
					*mat = mat4(1);
				}
		for (int i = 0; i < kids.size(); i++)
			kids[i]->play_animation(keyframenumber,animationname, t);

	}

	void change_animation(float keyframenumber, string animationname, string animationname2, float t) {
		if (animation.size() < 2) {
			cout << "Two Animations required.";
			return;
		}

		int frame = keyframenumber;								
		animation_per_bone *startAnim, *endAnim;				// define start and end animations
		for (int i = 0; i < animation.size(); i++) {
			if (animation[i]->name == animationname) {
				startAnim = animation[i];
			}
			if (animation[i]->name == animationname2) {
				endAnim = animation[i];
			}
		}
		
		frame = (int)keyframenumber % animation[0]->keyframes.size();	// only give frames within animation duration

		// ------- time warp animations so they have same duration --------
		int frame_1 = 0, frame_2 = 0;
		if (startAnim->keyframes.size() > endAnim->keyframes.size()) {					// get ratio of longer anim : shorter anim
			float scale = (float)startAnim->keyframes.size() / (float)endAnim->keyframes.size();
			//frame = (int)keyframenumber % endAnim->keyframes.size();	// only give frames within animation duration
			frame_1 = frame/scale;
			frame_2 = frame;
		}
		else {
			float scale = (float)endAnim->keyframes.size() / (float)startAnim->keyframes.size();
			//frame = (int)keyframenumber % startAnim->keyframes.size();	// only give frames within animation duration
			frame_1 = frame;
			frame_2 = frame/scale;
		}
	//	cout << "frame: " << frame << " frame 1: " << frame_1 << " frame 2: " << frame_2 << endl;

		// -------------- Animation 1 --------------
		if (startAnim->keyframes.size() > frame_1+2 && endAnim->keyframes.size() > frame_2+2)
		{
			quat qa_1 = startAnim->keyframes[frame_1].quaternion;	// intp quats between 2 frames of animation
			quat qb_1 = startAnim->keyframes[frame_1+1].quaternion;
			vec3 ta_1 = startAnim->keyframes[frame_1].translation;
			vec3 tb_1 = startAnim->keyframes[frame_1+1].translation;
			quat qr_1 = slerp(qa_1, qb_1, t);
			vec3 tr_1 = mix(ta_1, tb_1, vec3(t));
		
		// -------------- Animation 2 -------------- NOT SURE IF I WANT THIS APPROACH

			quat qa_2 = endAnim->keyframes[frame_2].quaternion;	// intp quats between 2 frames of animation
			quat qb_2 = endAnim->keyframes[frame_2+1].quaternion;
			vec3 ta_2 = endAnim->keyframes[frame_2].translation;
			vec3 tb_2 = endAnim->keyframes[frame_2+1].translation;
			quat qr_2 = slerp(qa_2, qb_2, t);
			vec3 tr_2 = mix(ta_2, tb_2, vec3(t));

		// ------------ Interpolate between two animations ---------------
			quat qr = slerp(qr_1, qr_2, t);
			vec3 tr = mix(tr_1, tr_2, vec3(t));

			// stop translation - figure moves in one spot
			if (name == "Humanoid:Hips")
				tr = vec3(0, 0, 0);

			mat4 R = mat4(qr);
			mat4 T = translate(mat4(1.0), tr);
			mat4 M = T * R;

			if (mat)
			{
				mat4 parentmat = mat4(1);
				if (parent)
					parentmat = *parent->mat;
				*mat = parentmat * M;
			}
			else
				*mat = mat4(1);

			for (int i = 0; i < kids.size(); i++)
				kids[i]->change_animation(keyframenumber, animationname, animationname2, t);
		}
	}


	//writes into the segment positions and into the animation index VBO
	void write_to_VBOs(vec3 origin, vector<vec3> &vpos, vector<unsigned int> &imat)
		{
		vpos.push_back(origin);
		vec3 endp = origin + pos;
		vpos.push_back(endp);

		if(parent)
			imat.push_back(parent->index);
		else
			imat.push_back(index);
		imat.push_back(index);

		for (int i = 0; i < kids.size(); i++)
			kids[i]->write_to_VBOs(endp, vpos, imat);
		}
	//searches for the correct animations as well as sets the correct element from the animation matrix array
	void set_animations(all_animations *all_anim,mat4 *matrices,int &animsize)
	{
		for (int ii = 0; ii < all_anim->animations.size(); ii++)
			if (all_anim->animations[ii].bone == name)
				animation.push_back(&all_anim->animations[ii]);

			mat = &matrices[index];	// every bone holds the address of its corresponding array
			animsize++;

		for (int i = 0; i < kids.size(); i++)
			kids[i]->set_animations(all_anim, matrices, animsize);

	}

	int getKeyFrameCount(string animationName) {
		for (auto animation : animation) {
			if (animation->name == animationName) {
				return animation->frames;
			}
		}
		return 1;
	}

	long long getDuration(std::string animationName) {
		for (auto animation : animation) {
			if (animation->name == animationName) {
				return animation->duration;
			}
		}
		return 0;
	}

};
int readtobone(string file,all_animations *all_animation, bone **proot);